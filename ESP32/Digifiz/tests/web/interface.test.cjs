const assert = require('node:assert/strict');
const { readFileSync, mkdtempSync, rmSync } = require('node:fs');
const { resolve, join } = require('node:path');
const { tmpdir } = require('node:os');
const { execFileSync } = require('node:child_process');
const { gunzipSync } = require('node:zlib');
const { test, before, after } = require('node:test');
const { chromium } = require('playwright-core');

// Serve the actual embedded page, with device endpoints mocked at the HTTP
// boundary. All DOM manipulation and application JavaScript run in Chromium.
const pages = {};
let pageDirectory;
let browser;
before(async () => {
    pageDirectory = mkdtempSync(join(tmpdir(), 'digifiz-pages-'));
    for (const variant of ['next', 'refiz']) {
        const output = join(pageDirectory, `${variant}.html`);
        execFileSync(process.env.PYTHON || 'python3', [
            resolve(__dirname, '../../main/tools/build_web_page.py'),
            '--source', resolve(__dirname, '../../main/data/digifiz_ws_connect.html'),
            '--output', output, '--variant', variant,
        ]);
        pages[variant] = readFileSync(output, 'utf8');
        assert.equal(gunzipSync(readFileSync(`${output}.gz`)).toString(), pages[variant]);
    }
    browser = await chromium.launch({
        executablePath: process.env.CHROMIUM_PATH || '/usr/bin/chromium',
        headless: true,
        args: ['--no-sandbox', '--disable-dev-shm-usage'],
    });
});
after(async () => {
    await browser?.close();
    if (pageDirectory) rmSync(pageDirectory, { recursive: true, force: true });
});

const inputFor = (page, name) => page.locator(`[id="input-${name}"]`);
const rowFor = (page, name) => page.locator('#table-container tbody tr')
    .filter({ has: inputFor(page, name) });
const readbackFor = (page, name) => rowFor(page, name).locator('td').nth(1);
const colorRows = page => page.locator('#colorSchemeTable tbody tr');

async function device(t, variant = 'next') {
    const defaultPayload = Array(65).fill(0);
    Object.assign(defaultPayload, {
        0: 0x07, 37: 0xF8, 38: 0x3B, 39: 0xFE, 40: 0x06,
        44: 0xF8, 45: 0xFB, 46: 0xEF, 47: 0xFD, 48: 0x06, 64: 0xF8,
    });
    const state = {
        params: [
            { name: 'alpha', value: 10, type: 'uint16_t', min: 0, max: 1000, step: 1 },
            { name: 'Speed filter', value: 20, type: 'float', min: 0, max: 1000, step: 0.25 },
            { name: 'enabled', value: 0, type: 'uint8_t', min: 0, max: 1 },
            { name: 'label', value: 'original', type: 'string', min: 0, max: 0 },
            { name: 'threshold', value: 30, type: 'uint16_t', min: 0, max: 1000 },
            { name: 'RPM color mode', value: 0, type: 'uint8_t', min: 0, max: 2,
              info: '0=no change, 1=constant color, 2=blinking' },
        ],
        posts: [],
        colorPosts: [],
        diagnosticPosts: [],
        diagnostic: { enabled: false, bit_count: 520,
            payload: [...defaultPayload], default_payload: defaultPayload },
        failDiagnosticWrites: false,
        failedNames: new Set(),
        reads: 0,
        beforeRead: null,
    };
    const context = await browser.newContext({ viewport: { width: 1400, height: 1000 } });
    const page = await context.newPage();
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    t.after(async () => {
        await context.close();
        assert.deepEqual(errors, [], 'page must not throw JavaScript errors');
    });
    await page.addInitScript(() => {
        window.alerts = [];
        window.confirmResult = true;
        window.alert = message => window.alerts.push(message);
        window.confirm = () => window.confirmResult;
        window.WebSocket = class {
            constructor() { setTimeout(() => this.onopen?.(), 0); }
            send() {}
        };
    });
    await page.route('**/*', async route => {
        const url = new URL(route.request().url());
        const json = data => route.fulfill({ json: data });
        if (url.pathname === '/') {
            return route.fulfill({ contentType: 'text/html', body: pages[variant] });
        }
        if (url.pathname === '/params' && url.searchParams.get('type') === 'params') {
            ++state.reads;
            const params = state.params.map(param => ({ ...param }));
            if (state.beforeRead) await state.beforeRead();
            return json({ params });
        }
        if (url.pathname === '/set_param') {
            const posted = route.request().postDataJSON();
            state.posts.push(posted);
            if (state.failedNames.has(posted.name)) {
                return route.fulfill({ status: 500, body: 'Parameter rejected' });
            }
            const param = state.params.find(param => param.name === posted.name);
            // Model the device's integer conversion in the readback value.
            param.value = param.type === 'float' ? Number(posted.value) : Math.trunc(Number(posted.value));
            return json({ status: 'ok' });
        }
        if (url.pathname === '/params' && url.searchParams.get('type') === 'color') {
            return json({ scheme: [10, 20, 30].map(end_segment => ({
                end_segment, type: 1, basecolor_enabled: 0, r: 12, g: 34, b: 56,
            })) });
        }
        if (url.pathname === '/set_color_scheme') {
            state.colorPosts.push(route.request().postDataJSON());
            return json({ status: 'ok' });
        }
        if (url.pathname === '/refiz_diagnostic' && variant === 'refiz') {
            if (route.request().method() === 'POST') {
                const posted = route.request().postDataJSON();
                state.diagnosticPosts.push(posted);
                if (state.failDiagnosticWrites) return route.fulfill({ status: 500, body: 'Write failed' });
                if (posted.reset || (posted.enabled && !state.diagnostic.enabled))
                    state.diagnostic.payload = [...state.diagnostic.default_payload];
                if (posted.payload) state.diagnostic.payload = [...posted.payload];
                state.diagnostic.enabled = posted.enabled;
            }
            return json(state.diagnostic);
        }
        return route.fulfill({ status: 404, body: 'Not mocked' });
    });
    await page.goto('http://digifiz.test/');
    return { page, state };
}

async function settings(page) {
    await page.getByRole('link', { name: 'Settings', exact: true }).click();
    await page.getByRole('button', { name: 'Load Parameters', exact: true }).click();
    await inputFor(page, 'alpha').waitFor();
}

async function waitForReadback(page, name, value) {
    await page.waitForFunction(({ name, value }) =>
        document.getElementById(`input-${name}`).closest('tr').cells[1].textContent === value,
    { name, value: String(value) });
}

test('Set refreshes readback while preserving all pending inputs and focus during the request', async t => {
    const { page, state } = await device(t);
    await settings(page);
    await inputFor(page, 'alpha').fill('12.9');
    await inputFor(page, 'Speed filter').fill('45.25');
    await inputFor(page, 'enabled').check();
    await inputFor(page, 'threshold').fill('');

    let release;
    let started;
    const paused = new Promise(resolve => { release = resolve; });
    const refreshStarted = new Promise(resolve => { started = resolve; });
    state.beforeRead = async () => { started(); await paused; };
    t.after(() => release());
    await rowFor(page, 'alpha').getByRole('button', { name: 'Set', exact: true }).click();
    await refreshStarted;
    await inputFor(page, 'label').fill('typing during refresh');
    await inputFor(page, 'label').evaluate(input => {
        window.editingInput = input;
        input.setSelectionRange(3, 8);
    });
    state.beforeRead = null;
    release();
    await waitForReadback(page, 'alpha', 12);
    assert.equal(await inputFor(page, 'Speed filter').inputValue(), '45.25');
    assert.equal(await inputFor(page, 'enabled').isChecked(), true);
    assert.equal(await inputFor(page, 'threshold').inputValue(), '');
    assert.equal(await inputFor(page, 'label').inputValue(), 'typing during refresh');
    assert.equal(await page.evaluate(() => document.activeElement === window.editingInput), true);
    assert.deepEqual(await inputFor(page, 'label').evaluate(input => [input.selectionStart, input.selectionEnd]), [3, 8]);

    await rowFor(page, 'Speed filter').getByRole('button', { name: 'Set', exact: true }).click();
    await waitForReadback(page, 'Speed filter', 45.25);
    await rowFor(page, 'enabled').getByRole('button', { name: 'Set', exact: true }).click();
    await waitForReadback(page, 'enabled', 1);
    assert.deepEqual(state.posts, [
        { name: 'alpha', value: '12.9' },
        { name: 'Speed filter', value: '45.25' },
        { name: 'enabled', value: 1 },
    ]);
});

test('a rejected Set preserves the submitted draft and unrelated edits', async t => {
    const { page, state } = await device(t);
    await settings(page);
    state.failedNames.add('alpha');
    await inputFor(page, 'alpha').fill('99');
    await inputFor(page, 'Speed filter').fill('88.5');
    await page.evaluate(() => setParam('alpha'));
    await page.waitForFunction(() => window.alerts.some(message => message.includes('Failed to set')));
    assert.equal(await inputFor(page, 'alpha').inputValue(), '99');
    assert.equal(await inputFor(page, 'Speed filter').inputValue(), '88.5');
    assert.equal(await readbackFor(page, 'alpha').textContent(), '10');
    assert.equal(state.reads, 1);
});

test('imported drafts remain available for individual Set buttons', async t => {
    const { page } = await device(t);
    await settings(page);
    await page.evaluate(() => { window.confirmResult = false; });
    await page.locator('#importParamsFile').setInputFiles({
        name: 'parameters.json', mimeType: 'application/json',
        buffer: Buffer.from(JSON.stringify({ params: [
            { name: 'alpha', value: 42 }, { name: 'Speed filter', value: 250.75 },
        ] })),
    });
    await page.waitForFunction(() => window.alerts.some(message => message.includes('populated')));
    await rowFor(page, 'alpha').getByRole('button', { name: 'Set', exact: true }).click();
    await waitForReadback(page, 'alpha', 42);
    assert.equal(await inputFor(page, 'Speed filter').inputValue(), '250.75');
    await rowFor(page, 'Speed filter').getByRole('button', { name: 'Set', exact: true }).click();
    await waitForReadback(page, 'Speed filter', 250.75);
});

test('partial bulk apply retains rejected drafts while refreshing accepted values', async t => {
    const { page, state } = await device(t);
    await settings(page);
    state.failedNames.add('Speed filter');
    await inputFor(page, 'alpha').fill('42');
    await inputFor(page, 'Speed filter').fill('250.75');
    await page.evaluate(() => applyImportedParameterValues(new Map([
        ['alpha', { type: 'uint16_t', value: 42 }],
        ['Speed filter', { type: 'float', value: 250.75 }],
    ])));
    await waitForReadback(page, 'alpha', 42);
    assert.equal(await readbackFor(page, 'Speed filter').textContent(), '20');
    assert.equal(await inputFor(page, 'Speed filter').inputValue(), '250.75');
});

test('explicit Load Parameters still replaces drafts with device values', async t => {
    const { page } = await device(t);
    await settings(page);
    await inputFor(page, 'alpha').fill('99');
    await inputFor(page, 'enabled').check();
    await page.evaluate(() => loadParams());
    assert.equal(await inputFor(page, 'alpha').inputValue(), '10');
    assert.equal(await inputFor(page, 'enabled').isChecked(), false);
});

test('color modes have named choices, preserve a pending mode, and submit its numeric code', async t => {
    const { page, state } = await device(t);
    await settings(page);
    const mode = inputFor(page, 'RPM color mode');
    assert.deepEqual(await mode.locator('option').allTextContents(), ['No change', 'Blinking', 'Constant color']);
    await mode.selectOption({ label: 'Blinking' });
    await inputFor(page, 'alpha').fill('50');
    await rowFor(page, 'alpha').getByRole('button', { name: 'Set', exact: true }).click();
    await waitForReadback(page, 'alpha', 50);
    assert.equal(await mode.inputValue(), '2');
    await rowFor(page, 'RPM color mode').getByRole('button', { name: 'Set', exact: true }).click();
    await waitForReadback(page, 'RPM color mode', 2);
    assert.deepEqual(state.posts.at(-1), { name: 'RPM color mode', value: '2' });
});

test('Next contains no diagnostic tab, panel, or diagnostic JavaScript', async t => {
    const { page } = await device(t, 'next');
    assert.equal(await page.getByRole('link', { name: 'Diagnostics', exact: true }).count(), 0);
    assert.equal(await page.locator('#Diagnostics').count(), 0);
    assert(!pages.next.includes('/refiz_diagnostic'));
    assert(!pages.next.includes('refizSegments'));
});

async function diagnostics(page) {
    await page.getByRole('link', { name: 'Diagnostics', exact: true }).click();
    await page.waitForFunction(() => document.querySelectorAll('#refizSegments input').length === 520);
}

async function enableDiagnostics(page) {
    await page.locator('#refizDiagnosticEnabled').check();
    await page.waitForFunction(() => !document.getElementById('refizApply').disabled);
}

async function applyDiagnostics(page) {
    await page.locator('#refizApply').click();
    await page.waitForFunction(() => !document.getElementById('refizApply').disabled);
}

async function checkedBits(page) {
    return page.locator('#refizSegments input:checked').evaluateAll(inputs =>
        inputs.map(input => Number(input.dataset.bit)));
}

function payloadBits(payload) {
    return Array.from({ length: 520 }, (_, bit) => bit).filter(bit => payload[bit >> 3] & (1 << (bit & 7)));
}

test('ReFIZ activation loads device defaults and edits raw housekeeping and upper payload bits', async t => {
    const { page, state } = await device(t, 'refiz');
    await diagnostics(page);
    assert.deepEqual(await checkedBits(page), payloadBits(state.diagnostic.default_payload));
    assert.equal(await page.locator('#refizApply').isEnabled(), false);
    await enableDiagnostics(page);
    assert.deepEqual(state.diagnosticPosts[0], { enabled: true, reset: true });
    assert.deepEqual(await checkedBits(page), payloadBits(state.diagnostic.default_payload));
    await page.locator('#refizSegments input[data-bit="0"]').uncheck();
    await page.locator('#refizSegments input[data-bit="1"]').uncheck();
    await page.locator('#refizSegments input[data-bit="264"]').check();
    await page.locator('#refizSegments input[data-bit="512"]').check();
    await page.locator('#refizSegments input[data-bit="519"]').uncheck();
    await applyDiagnostics(page);
    const expected = [...state.diagnostic.default_payload];
    expected[0] = 4;
    expected[33] = 1;
    expected[64] = 0x79;
    assert.deepEqual(state.diagnosticPosts[1], { enabled: true, payload: expected });
    assert.deepEqual(await checkedBits(page), payloadBits(expected));
});

test('ReFIZ selects/clears every payload bit and re-enabling starts from defaults', async t => {
    const { page, state } = await device(t, 'refiz');
    await diagnostics(page);
    await enableDiagnostics(page);
    await page.locator('#refizAllOn').click();
    assert.equal((await checkedBits(page)).length, 520);
    await applyDiagnostics(page);
    assert.deepEqual(state.diagnosticPosts[1], { enabled: true, payload: Array(65).fill(255) });
    await page.locator('#refizAllOff').click();
    assert.equal((await checkedBits(page)).length, 0);
    await applyDiagnostics(page);
    assert.deepEqual(state.diagnosticPosts[2], { enabled: true, payload: Array(65).fill(0) });
    await page.locator('#refizDiagnosticEnabled').uncheck();
    await page.waitForFunction(() => document.getElementById('refizDiagnosticStatus').textContent.startsWith('Diagnostics disabled'));
    assert.deepEqual(state.diagnosticPosts[3], { enabled: false, reset: false });
    await enableDiagnostics(page);
    assert.deepEqual(state.diagnostic.payload, state.diagnostic.default_payload);
    assert.deepEqual(await checkedBits(page), payloadBits(state.diagnostic.default_payload));
});

test('ReFIZ reload preserves an active edited pattern and Restore defaults stages the authoritative default', async t => {
    const { page, state } = await device(t, 'refiz');
    await diagnostics(page);
    state.diagnostic.enabled = true;
    state.diagnostic.payload = Array(65).fill(0xAA);
    await page.locator('#refizReload').click();
    await page.waitForFunction(() => !document.getElementById('refizApply').disabled);
    assert.deepEqual(await checkedBits(page), payloadBits(state.diagnostic.payload));
    assert.equal(state.diagnosticPosts.length, 0);
    await page.locator('#refizDefaults').click();
    assert.deepEqual(await checkedBits(page), payloadBits(state.diagnostic.default_payload));
    assert.deepEqual(state.diagnostic.payload, Array(65).fill(0xAA)); // Not applied yet.
    await applyDiagnostics(page);
    assert.deepEqual(state.diagnostic.payload, state.diagnostic.default_payload);
});

test('ReFIZ failed activation rolls back the toggle; failed applies preserve edits for retry', async t => {
    const { page, state } = await device(t, 'refiz');
    await diagnostics(page);
    state.failDiagnosticWrites = true;
    await page.locator('#refizDiagnosticEnabled').check();
    await page.waitForFunction(() => document.getElementById('refizDiagnosticStatus').textContent.startsWith('Could not'));
    assert.equal(await page.locator('#refizDiagnosticEnabled').isChecked(), false);
    assert.equal(await page.locator('#refizApply').isEnabled(), false);
    state.failDiagnosticWrites = false;
    await enableDiagnostics(page);
    await page.locator('#refizSegments input[data-bit="519"]').uncheck();
    state.failDiagnosticWrites = true;
    await applyDiagnostics(page);
    assert.equal(await page.locator('#refizSegments input[data-bit="519"]').isChecked(), false);
    assert.equal(state.diagnostic.payload[64], 0xF8);
    state.failDiagnosticWrites = false;
    await applyDiagnostics(page);
    assert.equal(state.diagnostic.payload[64], 0x78);
});

async function colors(page) {
    await page.getByRole('link', { name: 'Colors', exact: true }).click();
    await page.locator('#loadColorScheme').click();
    await page.waitForFunction(() => document.querySelectorAll('#colorSchemeTable tbody tr').length === 3);
}

const colorOrder = page => colorRows(page).locator('td:first-child input')
    .evaluateAll(inputs => inputs.map(input => input.value));

for (const direction of ['Above', 'Below']) {
    for (const index of [0, 1, 2]) {
        test(`Colors inserts ${direction.toLowerCase()} row ${index + 1} at the requested position`, async t => {
            const { page } = await device(t);
            await colors(page);
            await colorRows(page).nth(index).getByTitle(`Create New ${direction}`, { exact: true }).click();
            const expected = ['10', '20', '30'];
            expected.splice(index + (direction === 'Below' ? 1 : 0), 0, '');
            assert.deepEqual(await colorOrder(page), expected);
            assert.equal(await page.locator('#setColorScheme').isEnabled(), true);
        });
    }
}

test('Colors inserts relative to a moved row and sends the resulting order to the device', async t => {
    const { page, state } = await device(t);
    await colors(page);
    await colorRows(page).nth(1).getByTitle('Move Up', { exact: true }).click();
    assert.deepEqual(await colorOrder(page), ['20', '10', '30']);
    await colorRows(page).nth(0).getByTitle('Create New Below', { exact: true }).click();
    const inserted = colorRows(page).nth(1);
    await inserted.locator('td').nth(0).locator('input').fill('25');
    await inserted.locator('td').nth(1).locator('select').selectOption('3');
    await inserted.locator('input[type="color"]').fill('#123456');
    assert.deepEqual(await colorOrder(page), ['20', '25', '10', '30']);
    await page.locator('#setColorScheme').click();
    await page.waitForFunction(() => window.alerts.includes('All color schemes updated successfully!'));
    assert.deepEqual(state.colorPosts.map(post => [post.scheme[0].end_segment, post.scheme[0].i]),
        [[20, 0], [25, 1], [10, 2], [30, 3]]);
    assert.deepEqual(state.colorPosts[1].scheme[0], {
        end_segment: 25, type: 3, basecolor_enabled: 0, r: 18, g: 52, b: 86, i: 1,
    });
});

test('Colors append, move, and delete still work, including the empty-table state', async t => {
    const { page } = await device(t);
    await page.getByRole('link', { name: 'Colors', exact: true }).click();
    assert.equal(await page.locator('#setColorScheme').isEnabled(), false);
    await page.locator('#addColorSegment').click();
    await colorRows(page).nth(0).locator('td:first-child input').fill('10');
    await colorRows(page).nth(0).getByTitle('Create New Below', { exact: true }).click();
    await colorRows(page).nth(1).locator('td:first-child input').fill('20');
    await colorRows(page).nth(1).getByTitle('Move Up', { exact: true }).click();
    assert.deepEqual(await colorOrder(page), ['20', '10']);
    await colorRows(page).nth(0).getByTitle('Move Down', { exact: true }).click();
    assert.deepEqual(await colorOrder(page), ['10', '20']);
    await colorRows(page).nth(0).getByTitle('Delete', { exact: true }).click();
    await colorRows(page).nth(0).getByTitle('Delete', { exact: true }).click();
    assert.equal(await colorRows(page).count(), 0);
    assert.equal(await page.locator('#setColorScheme').isEnabled(), false);
});
