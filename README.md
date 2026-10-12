# PixelClear Studio — Dashboard + C Backend

This project combines the PixelClear Studio dashboard with a small Windows C web server.

## Features
- Original HTML/CSS/JavaScript dashboard.
- C backend serves `index.html` at `http://127.0.0.1:8000/`.
- `GET /api/status` health endpoint used by the dashboard's backend indicator.
- Existing background removal runs in the browser using the JavaScript model loaded from a public CDN. The first run needs an internet connection.

**Important:** The C server hosts the dashboard and reports its status; it does not run the background-removal model itself. The dashboard's existing browser-side model handles image removal.

## Requirements
- Windows
- GCC/MinGW installed and available in PATH (`gcc --version` should work)
- Internet access for the browser to download the background-removal library/model

## Run in VS Code
1. Extract this folder.
2. Open the `pixelclear-studio-working` folder in VS Code.
3. Choose **Terminal → New Terminal**.
4. Compile the server from the project root:

   ```powershell
   gcc backend/backend.c -o backend.exe -lws2_32
   ```

5. Start it:

   ```powershell
   .\backend.exe
   ```

6. Open `http://127.0.0.1:8000/` in your browser. Do not open `index.html` as a `file://` URL.
7. The bottom-right status indicator should show **C backend: connected**.
8. You can check the API at `http://127.0.0.1:8000/api/status`.

## One-click Windows helper
Double-click `run_backend.bat`. It compiles the C server and starts it. If GCC is not installed, use the compile command above after installing GCC/MinGW.

## Troubleshooting
- **`gcc` is not recognized:** Install MinGW-w64/WinLibs and add its `bin` folder to PATH, then restart VS Code.
- **Port 8000 could not be opened:** Close the other app using port 8000, or change `PORT` in `backend/backend.c` and the matching dashboard address if you have hard-coded one.
- **Backend offline badge:** Run the server and open the dashboard through `http://127.0.0.1:8000/`.
- **Background removal fails:** Check your internet connection; the browser loads the removal library/model from a public CDN. Look in the browser developer console for details.
