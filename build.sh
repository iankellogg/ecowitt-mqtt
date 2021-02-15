#!/bin/bash
gcc *.c mqtt/*.c -o ~/public_html/cgi-bin/ecowitt.cgi -lcgic -lpaho-embed-mqtt3c
