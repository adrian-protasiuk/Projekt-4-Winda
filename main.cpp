// main.cpp
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include "logikaWindy.cpp"
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;

#define BUTTON_ID_BASE 1000

Winda winda;
vector<unique_ptr<Pasazer>> pasazerowie;


void DrawWinda(Graphics& g)
{
    SolidBrush windaBrush(Color(150, 150, 255));
    SolidBrush pasazerBrush(Color(0, 180, 0));
    SolidBrush waitingBrush(Color(255, 100, 100));
    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 12, FontStyleRegular, UnitPixel);
    SolidBrush textBrush(Color(0, 0, 0));

    int baseX = 50; // winda z lewej
    int baseY = 400; // pi�tro 0
    int floorHeight = 60;
    int width = 50;
    int height = 50;

    // Waga
    wstring wagaStr = L"Waga: " + to_wstring(winda.waga) + L" kg";
    g.DrawString(wagaStr.c_str(), -1, &font, PointF(10, 10), &textBrush);

    // Rysuj wind�
    int wy = baseY - ((winda.pietro + 1) * floorHeight);
    g.FillRectangle(&windaBrush, baseX, wy, width, height);

    // Pasa�erowie w windzie
    int i = 0;
    for (int idx : winda.vectorPasazerow) {
        const Pasazer& p = *pasazerowie[idx];
        int py = wy + 5 + (i / 4) * 10;
        int px = baseX + 5 + (i % 4) * 10;
        g.FillRectangle(&pasazerBrush, px, py, 8, 8);
        i++;
    }

    // Pi�tra i pasa�erowie czekaj�cy
    Pen pen(Color(0, 0, 0));
    int panelX = baseX + 100; // po prawej od windy
    int personSize = 8;

    for (int f = 0; f < 5; ++f) {
        int fy = baseY - f * floorHeight;

        // pi�tro linia
        g.DrawLine(&pen, baseX, fy, panelX + 160, fy);

        // tekst pi�tra
        wstring pietroText = L"Pietro " + to_wstring(f);
        g.DrawString(pietroText.c_str(), -1, &font, PointF(panelX, fy - 15), &textBrush);

        // pasa�erowie czekaj�cy
        int count = 0;
        for (const auto& ptr : pasazerowie) {
            const Pasazer& p = *ptr;
            if (p.stan == czeka && p.pietroStart == f) {
                int px = panelX + (count % 4) * (personSize + 2);
                int py = fy - height + (count / 4) * (personSize + 2);
                g.FillRectangle(&waitingBrush, px, py, personSize, personSize);
                count++;
            }
        }

    }
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 1000, NULL);

        for (int floor = 0; floor < 5; ++floor) {
            for (int cel = 0; cel < 5; ++cel) {

                std::wcout << L"Tworze przycisk: z " << floor << L" do " << cel
                    << L", btnId=" << (BUTTON_ID_BASE + floor * 10 + cel) << std::endl;


                if (cel == floor) continue;
                wchar_t label[16];
                swprintf(label, 16, L"Do %d", cel);
                int btnId = BUTTON_ID_BASE + floor * 10 + cel;

                CreateWindow(L"BUTTON", label,
                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    180 + cel * 45, // x
                    400 - floor * 60, // y
                    40, 20, hWnd, (HMENU)btnId, NULL, NULL);
            }
        }
        return 0;

    case WM_COMMAND:
    {
        int btnId = LOWORD(wParam);
        if (btnId >= BUTTON_ID_BASE && btnId < BUTTON_ID_BASE + 500) {
            int floor = (btnId - BUTTON_ID_BASE) / 10;
            int cel = (btnId - BUTTON_ID_BASE) % 10;

            // Zabezpieczenie: nie dodawaj identycznego pasa�era
            bool juzIstnieje = false;
            for (const auto& ptr : pasazerowie) {
                const Pasazer& p = *ptr;
                if (p.pietroStart == floor && p.pietroKoniec == cel && p.stan != dojechal) {
                    juzIstnieje = true;
                    break;
                }
            }


            if (!juzIstnieje) {
                pasazerowie.push_back(make_unique<Pasazer>(floor, cel));
                std::cout << "[DODANO] Pasazer: start=" << floor << ", cel=" << cel << std::endl;
            }
            else {
                std::cout << "[POMINIETO] Duplikat pasazera: start=" << floor << ", cel=" << cel << std::endl;
            }

            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;
    }


    case WM_TIMER:
        for (auto& p : pasazerowie) winda.wezwij(*p);
        for (size_t i = 0; i < pasazerowie.size(); ++i)
            winda.odbierz(*pasazerowie[i], static_cast<int>(i));

        winda.ruch();
        winda.odstaw();
        winda.pierwszyRuchJesliPotrzeba();
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        {
            Graphics g(hdc);
            RECT winRect;
            GetClientRect(hWnd, &winRect);
            int width = winRect.right - winRect.left;
            int height = winRect.bottom - winRect.top;
            SolidBrush whiteBrush(Color(255, 255, 255));
            g.FillRectangle(&whiteBrush, Rect(0, 0, width, height));
            DrawWinda(g);
        }
        EndPaint(hWnd, &ps);
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SymulacjaWindy";

    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);

    RegisterClass(&wc);
    HWND hWnd = CreateWindowEx(0, L"SymulacjaWindy", L"Symulator Windy", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}