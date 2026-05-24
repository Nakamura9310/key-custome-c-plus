#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define NOMINMAX

#include <windows.h>
#include <shellapi.h>
#include <algorithm>

HHOOK kbdHook;
HHOOK mouseHook;
bool f13_pressed = false;
bool sending_scroll = false;

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define TRAY_ICON_UID 1

NOTIFYICONDATA nid;
HWND hwnd;
HMENU hMenu;
HANDLE hMutex; // 多重起動防止用

//========================= キーボードフック =========================
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            switch (p->vkCode) {
                case VK_F13:
                    f13_pressed = true;
                    return 1; // F13キーイベントをキャプチャしてシステムに渡さない
                // Home
                case 'A':
                    if (f13_pressed) {
                        keybd_event(VK_HOME, 0, 0, 0);
                        keybd_event(VK_HOME, 0, KEYEVENTF_KEYUP, 0);
                        return 1;
                    }
                    break;
                // End
                case 'E':
                    if (f13_pressed) {
                        keybd_event(VK_END, 0, 0, 0);
                        keybd_event(VK_END, 0, KEYEVENTF_KEYUP, 0);
                        return 1;
                    }
                    break;
                // Delete
                case 'D':
                    if (f13_pressed) {
                        keybd_event(VK_DELETE, 0, 0, 0);
                        keybd_event(VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
                        return 1;
                    }
                    break;
                // Backspace
                case 'C':
                    if (f13_pressed) {
                        keybd_event(VK_BACK, 0, 0, 0);
                        keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
                        return 1;
                    }
                    break;
                // Left Arrow
                case 'J':
                    if (f13_pressed) {
                        keybd_event(VK_LEFT, 0, 0, 0);
                        keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
                        return 1;
                    }
                    break;
                // Up Arrow
                case 'I':
                    if (f13_pressed) {
                        keybd_event(VK_UP, 0, 0, 0);
                        keybd_event(VK_UP, 0, KEYEVENTF_KEYUP, 0);
                        return 1;
                    }
                    break;
                // Down Arrow
                case 'K':
                    if (f13_pressed) {
                        keybd_event(VK_DOWN, 0, 0, 0);
                        keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0);
                        return 1;
                    }
                    break;
                // Right Arrow
                case 'L':
                    if (f13_pressed) {
                        keybd_event(VK_RIGHT, 0, 0, 0);
                        keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
                        return 1;
                    }
                    break;
                // F7=カタカナ変換
                case 'N':
                    if (f13_pressed) {
                        keybd_event(VK_F7, 0, 0, 0);
                        keybd_event(VK_F7, 0, KEYEVENTF_KEYUP, 0);
                        return 1;
                    }
                    break;
                // 縮小 (F13 + S)
                case 'S':
                    if (f13_pressed) {
                        HWND target = GetForegroundWindow();
                        if (target && target != hwnd) {
                            RECT rc;
                            if (GetWindowRect(target, &rc)) {
                                int w = rc.right - rc.left;
                                int h = rc.bottom - rc.top;
                                int dw = -50; // 横幅の増減量（負で縮小）
                                int dh = -30; // 高さの増減量
                                int newW = std::max(100, w + dw);
                                int newH = std::max(50, h + dh);
                                SetWindowPos(target, NULL, rc.left, rc.top, newW, newH, SWP_NOZORDER | SWP_NOACTIVATE);
                            }
                        }
                        return 1;
                    }
                    break;
                // 拡大 (F13 + F)
                case 'F':
                    if (f13_pressed) {
                        HWND target = GetForegroundWindow();
                        if (target && target != hwnd) {
                            RECT rc;
                            if (GetWindowRect(target, &rc)) {
                                int w = rc.right - rc.left;
                                int h = rc.bottom - rc.top;
                                int dw = 50; // 横幅の増減量
                                int dh = 30; // 高さの増減量
                                int newW = std::min(4096, w + dw);
                                int newH = std::min(2160, h + dh);
                                SetWindowPos(target, NULL, rc.left, rc.top, newW, newH, SWP_NOZORDER | SWP_NOACTIVATE);
                            }
                        }
                        return 1;
                    }
                    break;
            }
        }

        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (p->vkCode == VK_F13) {
                f13_pressed = false;
            }
        }
    }

    return CallNextHookEx(kbdHook, nCode, wParam, lParam);
}

//========================= マウスフック =========================
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && f13_pressed && !sending_scroll && wParam == WM_MOUSEWHEEL) {
        MSLLHOOKSTRUCT* p = (MSLLHOOKSTRUCT*)lParam;
        int delta = GET_WHEEL_DELTA_WPARAM(p->mouseData);
        sending_scroll = true;

        // PageUp/PageDown を複数回送信（調整可能）
        for (int i = 0; i < 1; ++i) { // 1回だけ送信。個々の数値を変更すれば大きくスクロールできます。
            if (delta > 0) {
                keybd_event(VK_PRIOR, 0, 0, 0);         // PageUp
                keybd_event(VK_PRIOR, 0, KEYEVENTF_KEYUP, 0);
            } else {
                keybd_event(VK_NEXT, 0, 0, 0);           // PageDown
                keybd_event(VK_NEXT, 0, KEYEVENTF_KEYUP, 0);
            }
            Sleep(10); // 少し間隔を空けて安定させる
        }

        sending_scroll = false;
        return 1;  // 元のスクロールイベントをブロック
    }

    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

//========================= ウィンドウプロシージャ =========================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TRAYICON && lParam == WM_RBUTTONUP) {
        POINT p;
        GetCursorPos(&p);
        SetForegroundWindow(hWnd);  // これがないとメニューがすぐ消えることがある
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, p.x, p.y, 0, hWnd, NULL);
    } else if (msg == WM_COMMAND && LOWORD(wParam) == ID_TRAY_EXIT) {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
    } else if (msg == WM_DESTROY) {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

//========================= エントリーポイント =========================
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    // 二重起動チェック：名前付きミューテックスを作る
    hMutex = CreateMutex(NULL, TRUE, L"F13KeyCustomMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, L"すでに起動しています。", L"F13 Key Custom", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    // ウィンドウクラス登録
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TrayWindowClass";
    RegisterClass(&wc);

    // メッセージウィンドウ作成（非表示）
    hwnd = CreateWindowEx(
        0,                     // 拡張スタイルなし
        L"TrayWindowClass",   // ウィンドウクラス名（さっき RegisterClass() で登録したやつ）
        L"",                   // ウィンドウタイトル（なし）
        0,                     // スタイル（表示しない）
        0, 0, 0, 0,            // x, y, width, height（全部ゼロ）
        NULL, NULL,            // 親ウィンドウなし、メニューなし
        hInstance,             // アプリケーションインスタンス
        NULL                   // 追加データなし
    );

    // HICON型の変数を用意（hCustomIcon はグローバルじゃなくてもOK）
    HICON hCustomIcon = (HICON)LoadImage(
        NULL,                // モジュールハンドル（NULLで外部ファイル）
        L"icon.ico",         // ファイル名（実行ファイルと同じ場所に置く）
        IMAGE_ICON,          // アイコンを読み込む
        0, 0,                // サイズ（0でデフォルトサイズ）
        LR_LOADFROMFILE | LR_DEFAULTSIZE  // ファイルから読み込み＋自動サイズ
    );
    // 読み込めなかったらフォールバック（Windows標準アイコン）
    if (!hCustomIcon) {
        hCustomIcon = LoadIcon(NULL, IDI_APPLICATION);
    }

    // タスクトレイにアイコン追加
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_UID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hCustomIcon; // カスタムアイコン
    lstrcpy(nid.szTip, L"keycustom.exe");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // メニュー作成（右クリック用）
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"終了");

    // フック設定
    kbdHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);

    // メッセージループ
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // クリーンアップ
    UnhookWindowsHookEx(kbdHook);
    UnhookWindowsHookEx(mouseHook);
    DestroyMenu(hMenu);
    Shell_NotifyIcon(NIM_DELETE, &nid); // タスクトレイからアイコン削除（必須）
    if (hCustomIcon) DestroyIcon(hCustomIcon); // 最後に解放（忘れるとメモリリークする）
    if (hMutex) CloseHandle(hMutex); // ミューテックス解放（重複起動対策）

    return 0;
}
