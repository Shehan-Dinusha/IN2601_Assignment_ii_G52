@echo off
docker build -t in2601-builder .
if %errorlevel% neq 0 exit /b %errorlevel%
docker run --rm -v "%cd%:/src" in2601-builder

