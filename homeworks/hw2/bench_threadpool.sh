#!/bin/bash
# Install artillery:
# npm install -g artillery
artillery quick -k --count 10000 -n 200 http://localhost:8000
