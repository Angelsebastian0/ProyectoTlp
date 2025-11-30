#!/usr/bin/env bash
set -e
BIN=bin/../bin/motor_integration
if [ ! -f ./bin/motor_integration ]; then
  echo "Compilando..."
  make
fi
if [ -z "$1" ]; then
  echo "Uso: ./run.sh games/tetris.ast.json"
  exit 1
fi
./bin/motor_integration "$1"