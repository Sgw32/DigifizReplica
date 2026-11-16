"""Release packaging regressions. Run with the ESP-IDF Python (includes PyYAML):

python -m unittest discover -s .github/tests -v

Executes the workflow's real shell steps against temporary build outputs and
a fake gh executable. No GitHub release or repository refs are modified.
"""

import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest
import zipfile

import yaml


WORKFLOW = Path(__file__).resolve().parents[1] / "workflows/deploy_releases.yml"
DOCUMENT = yaml.load(WORKFLOW.read_text(), Loader=yaml.BaseLoader)
STEPS = {step["name"]: step for step in DOCUMENT["jobs"]["build-and-deploy"]["steps"]}


class ReleaseWorkflowTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="digifiz-release-test-")
        self.root = Path(self.temporary.name)
        self.project = self.root / "ESP32/Digifiz"
        self.output_names = ["Digifiz.bin", "Digifiz.elf", "bootloader/bootloader.bin",
                             "partition_table/partition-table.bin", "ota_data_initial.bin"]
        for directory, variant in [("build", "next"), ("build-refiz", "refiz")]:
            for name in self.output_names:
                path = self.project / directory / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(f"{variant}:{name}".encode())
        (self.project / "main").mkdir()
        (self.project / "main/setup_refiz.h").write_text("ReFIZ setup\n")
        (self.root / "build/setup_g2").mkdir(parents=True)
        (self.root / "build/setup_g2/Digifiz.hex").write_text("Arduino fixture\n")
        self.log = self.root / "gh-calls.jsonl"
        fake_bin = self.root / "fake-bin"
        fake_bin.mkdir()
        gh = fake_bin / "gh"
        gh.write_text("""#!/usr/bin/env python3
import json, os, pathlib, sys
args = sys.argv[1:]
with open(os.environ['TEST_GH_LOG'], 'a') as log:
    log.write(json.dumps(args) + '\\n')
if args[:2] == ['release', 'view']:
    sys.exit(0 if os.environ.get('TEST_RELEASE_EXISTS') == '1' else 1)
if args[:2] == ['release', 'upload']:
    for filename in args[3:]:
        if not filename.startswith('--'):
            assert pathlib.Path(filename).stat().st_size > 0
    if os.environ.get('TEST_FAIL_UPLOAD') == '1':
        sys.exit(1)
""")
        gh.chmod(0o755)
        self.env = dict(os.environ, PATH=f"{fake_bin}:{os.environ['PATH']}",
                        RUNNER_TEMP=str(self.root), TEST_GH_LOG=str(self.log),
                        RELEASE_TAG="v1.0.24", GITHUB_SHA="f" * 40)

    def tearDown(self):
        # The permission test deliberately makes only its own fixture read-only.
        for path in (self.project / "build").rglob("*"):
            path.chmod(0o755 if path.is_dir() else 0o644)
        (self.project / "build").chmod(0o755)
        self.temporary.cleanup()

    def run_step(self, name, success=True):
        result = subprocess.run(["bash", "-e", "-o", "pipefail", "-c", STEPS[name]["run"]],
                                cwd=self.root, env=self.env, capture_output=True, text=True)
        if success:
            self.assertEqual(result.returncode, 0, result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0)
        return result

    def calls(self):
        return [json.loads(line) for line in self.log.read_text().splitlines()] if self.log.exists() else []

    def package(self):
        self.run_step("Zip All Artifacts")
        self.run_step("Preserve Digifiz Next Build")
        self.run_step("Select ReFIZ setup")
        self.run_step("Package ReFIZ firmware")

    def test_release_version_uses_trigger_tag_without_fetching_git_tags(self):
        environment_file = self.root / "github-env"
        self.env["GITHUB_ENV"] = str(environment_file)
        # This fixture is not a Git checkout: no tag fetch or describe is needed.
        self.run_step("Set VERSION from release tag")
        self.assertEqual(environment_file.read_text(), "VERSION=v1.0.24\n")

    def test_separate_refiz_build_and_read_only_next_outputs(self):
        if os.geteuid() == 0:
            self.skipTest("Permission reproduction requires the normal unprivileged task user")
        build = STEPS["Build ReFIZ firmware"]["with"]
        self.assertEqual(shlex.split(build["command"]), ["idf.py", "-B", "build-refiz", "build"])
        for path in (self.project / "build").rglob("*"):
            path.chmod(0o555 if path.is_dir() else 0o444)
        (self.project / "build").chmod(0o555)
        # Reproduce the previous failing cleanup on an exact temporary fixture.
        denied = subprocess.run(["rm", "-rf", str(self.project / "build")], capture_output=True)
        self.assertNotEqual(denied.returncode, 0)
        self.package()  # New workflow never needs to remove or modify Next outputs.
        self.assertEqual((self.project / "main/setup.h").read_text(), "ReFIZ setup\n")
        self.assertEqual((self.root / "release/digifiz-next/bootloader.bin").read_bytes(),
                         b"next:bootloader/bootloader.bin")
        self.assertEqual((self.root / "release/digifiz-next/partition_table.bin").read_bytes(),
                         b"next:partition_table/partition-table.bin")
        with zipfile.ZipFile(self.root / "release/refiz-firmware.zip") as archive:
            self.assertEqual(set(archive.namelist()), {"Digifiz.bin", "Digifiz.elf", "bootloader.bin",
                                                     "partition-table.bin", "ota_data_initial.bin"})
            self.assertEqual(archive.read("Digifiz.bin"), b"refiz:Digifiz.bin")

    def test_new_release_is_published_only_after_seven_valid_assets(self):
        self.package()
        self.run_step("Publish complete release")
        calls = self.calls()
        self.assertEqual([call[1] for call in calls], ["view", "create", "upload", "edit"])
        self.assertIn("--draft", calls[1])
        self.assertIn("--verify-tag", calls[1])
        self.assertEqual(len([arg for arg in calls[2][3:] if not arg.startswith("--")]), 7)
        self.assertIn("release/digifiz-next/bootloader.bin", calls[2])
        self.assertIn("release/digifiz-next/partition_table.bin", calls[2])
        self.assertIn("--draft=false", calls[3])

    def test_existing_release_is_staged_and_replaced_without_duplicate_creation(self):
        self.env["TEST_RELEASE_EXISTS"] = "1"
        self.package()
        self.run_step("Publish complete release")
        calls = self.calls()
        self.assertEqual([call[1] for call in calls], ["view", "edit", "upload", "edit"])
        self.assertIn("--draft=true", calls[1])
        self.assertIn("--clobber", calls[2])
        self.assertIn("--draft=false", calls[3])

    def test_missing_asset_makes_no_release_api_calls(self):
        self.package()
        (self.root / "release/digifiz-next/bootloader.bin").unlink()
        self.run_step("Publish complete release", success=False)
        self.assertEqual(self.calls(), [])

    def test_failed_upload_does_not_publish_partial_release(self):
        self.env["TEST_RELEASE_EXISTS"] = "1"
        self.env["TEST_FAIL_UPLOAD"] = "1"
        self.package()
        self.run_step("Publish complete release", success=False)
        self.assertEqual([call[1] for call in self.calls()], ["view", "edit", "upload"])


if __name__ == "__main__":
    unittest.main()
