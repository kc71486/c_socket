#!/bin/bash
gcc -lpthread -o ./bin/server ./src/server.c
gcc -lpthread -o ./bin/client ./src/client.c
