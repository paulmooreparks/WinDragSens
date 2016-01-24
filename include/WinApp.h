#pragma once

#include "windows.h"
#include "windowsx.h"
#include "Window.h"
#include <exception>

namespace SmartSam {
   namespace AppKit {
      class WinAppException : public std::exception
      {
         public:
         int msgID;

         WinAppException(int msgIDInit) : msgID(msgIDInit) {
         }
      };

      class WinApp : public Window
      {
         protected:
            HACCEL hAccelTable;
            WCHAR title[100];
            WCHAR windowClass[100];
            LPSTR pCmdLine;
            int showCmd;

         public:
            WinApp(HINSTANCE hInstanceInit, LPSTR lpCmdLine, int nShowCmd)
               : Window(hInstanceInit), pCmdLine(lpCmdLine), showCmd(nShowCmd) {
            }

            virtual ~WinApp() {
            }


#pragma warning(push)
#pragma warning(disable: 4706)
            virtual int Run() {
               MSG msg = {};
               PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

               /* Call Initialize on construction and Uninitialize on destruction,
               thus guaranteeing that both are called no matter how we exit. */
               Initializer init(this);

               if (init) {
                  BOOL msgRet = FALSE;

                  while (msgRet = GetMessage(&msg, NULL, 0, 0) != 0) {
                     if (msgRet == -1) {
                        MessageBox(NULL, L"Message retrieval failed", L"Error", MB_OK);
                        return -1;
                     }
                     else {
                        if (!IsDialogMessage(GetHWND(), &msg) && !DoTranslateAccelerator(GetHWND(), &msg)) {
                           TranslateMessage(&msg);
                           DispatchMessage(&msg);
                        }
                     }
                  }

                  return 0;
               }
               else {
                  return -1;
               }
            }
#pragma warning(pop)


         protected:
            /* Override to perform any initialization that must occur when the app starts
            to run. */
            virtual bool Initialize() {
               return true;
            }

            /* When overriding, DO NOT assume that Initialize completed successfully.
            Test all objects and member variables as necessary while uninitializing. */
            virtual void Uninitialize() {
            }

            /* RAII wrapper to guarantee initialization and uninitialization. */
            class Initializer
            {
               private:
                  WinApp* pApp;
                  bool isInitialized;

               public:
                  Initializer(WinApp* pAppInit) : pApp(pAppInit), isInitialized(false) {
                     if (pApp) {
                        isInitialized = pApp->Initialize();
                     }
                  }

                  operator bool() const {
                     return isInitialized;
                  }

                  ~Initializer() {
                     if (pApp) {
                        pApp->Uninitialize();
                     }
                  }
            };

            friend class Initializer;

            virtual void PreRegisterClass(WNDCLASSEX& wc) {
               Window::PreRegisterClass(wc);
               wc.hCursor = LoadCursor(NULL, IDC_ARROW);
               wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
               wc.lpszClassName = windowClass;
            }

            /* Override this method to provide custom message handling. */
            virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
               switch (msg) {
               case WM_DESTROY:
                  PostQuitMessage(0);
                  break;

               default:
                  return Window::WndProc(msg, wParam, lParam);
               }

               return 0;
            }

            /* Translate accelerators */
            virtual int WINAPI DoTranslateAccelerator(
               __in HWND hwnd,
               __in LPMSG pMsg) {
               return TranslateAccelerator(hwnd, hAccelTable, pMsg);
            }

      };
   }
}
