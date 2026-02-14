#!/usr/bin/env bash

# Build Digifiz Replica Arduino firmware using Arduino CLI.
# The script installs arduino-cli if it is not available and
# then compiles all firmware variants.

set -e

REPO_DIR="$(dirname "$0")/.."
cd "$REPO_DIR"

# Ensure arduino-cli is available
if ! command -v arduino-cli >/dev/null 2>&1; then
    echo "arduino-cli not found; installing..." >&2
    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
    export PATH="$HOME/bin:$PATH"
fi

# Install/Update AVR core
arduino-cli core update-index
arduino-cli core install arduino:avr

# Create basic arduino-cli config
mkdir -p ~/.arduino15
cat > ~/.arduino15/arduino-cli.yaml <<'CFG'
board_manager:
  additional_urls: []
daemon:
  port: "50051"
logging:
  file: ""
  format: text
  level: info
metrics:
  addr: ""
  enabled: false
sketchbook_path: "$HOME/Arduino"
allow_unsafe_install: true
CFG

# Firmware variants to build
VARIANTS=${VARIANTS:-"setup_audi_red setup_audi_green setup_g2 setup_g2_uk setup_g2_usa setup_g2_diesel"}

ORIG=A2560_BoardR2/Digifiz/setup.h
BACKUP="$ORIG.orig"
cp "$ORIG" "$BACKUP" 2>/dev/null || true
mkdir -p build

for VAR in $VARIANTS; do
    echo "Building $VAR..."
    cp "A2560_BoardR2/Digifiz/${VAR}.h" "$ORIG"
    arduino-cli compile --fqbn arduino:avr:mega A2560_BoardR2/Digifiz/ --output-dir "build/$VAR"
    echo "Finished $VAR"
done

mv "$BACKUP" "$ORIG"

echo "Build artifacts located in ./build/"
