#!/bin/bash

clang obsidian_image_extractor.c -o obsidian_image_extractor && ./obsidian_image_extractor test/The\ Necessary\ Theory\ -\ Part\ 1.md test/ /tmp/ malware_analysis
