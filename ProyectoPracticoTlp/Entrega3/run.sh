#!/usr/bin/env bash
set -e
if [ ! -f ./bin/motor_integration ]; then
  echo "Compilando..."
  make
fi
if [ -z "$1" ]; then
  echo "Uso: ./run.sh games/snake.script"
  exit 1
fi
./bin/motor_integration "$1"
