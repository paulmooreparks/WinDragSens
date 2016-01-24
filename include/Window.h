#pragma once

#include "windows.h"
#include <exception>

namespace SmartSam {
   namespace AppKit {
      class WindowException : public std::exception
      {
         public:
         int msgID;

         WindowException(int msgIDInit) : msgID(msgIDInit) {
         }
      };

      class Window
      {
         private:
            HINSTANCE hInstance;
            HWND thisHwnd;

            public:
            Window(HINSTANCE hInstanceInit) : hInstance(hInstanceInit) {
            }

            virtual ~Window() {
            }

            HWND GetHWND() const {
               return thisHwnd;
            }

            operator HWND() const {
               return thisHwnd;
            }


            HINSTANCE GetInstance() const {
               return hInstance;
            }


            /* This static member of the App class is called whenever a window message is
            generated for the main window. */
            static LRESULT CALLBACK BaseWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
               Window* pWin = 0;

               if (msg == WM_NCCREATE) {
                  LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
                  pWin = reinterpret_cast<Window*>(lpcs->lpCreateParams);
                  pWin->thisHwnd = hwnd;
                  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pWin));
               }
               else {
                  LONG_PTR gwlud = GetWindowLongPtr(hwnd, GWLP_USERDATA);
                  pWin = reinterpret_cast<Window*>(gwlud);
               }

               if (pWin) {
                  return pWin->WndProc(msg, wParam, lParam);
               }

               return DefWindowProc(hwnd, msg, wParam, lParam);
            }

            /* This static member of the App class is called whenever a window message is
            generated for the main window. */
            static INT_PTR CALLBACK BaseDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
               Window* pWin = 0;

               if (msg == WM_INITDIALOG) {
                  pWin = reinterpret_cast<Window*>(lParam);
                  pWin->thisHwnd = hwnd;
                  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pWin));
               }
               else {
                  LONG_PTR gwlud = GetWindowLongPtr(hwnd, GWLP_USERDATA);
                  pWin = reinterpret_cast<Window*>(gwlud);
               }

               if (pWin) {
                  return pWin->WndProc(msg, wParam, lParam);
               }

               return DefWindowProc(hwnd, msg, wParam, lParam);
            }

         protected:

            bool DoRegisterClass() {
               WNDCLASSEX wc = { 0 };

               wc.cbSize = sizeof(WNDCLASSEX);
               wc.lpfnWndProc = (WNDPROC)SmartSam::AppKit::Window::BaseWndProc;;
               wc.hInstance = hInstance;

               PreRegisterClass(wc);

               return !!RegisterClassEx(&wc);
            }

            virtual void PreRegisterClass(WNDCLASSEX& wc) {
            }

            HWND DoCreateWindow(
               DWORD exStyle,
               LPCTSTR pClassName,
               LPCTSTR pWindowName,
               DWORD style,
               int x,
               int y,
               int cx,
               int cy,
               HWND hwndParent,
               HMENU hmenu) {
               return CreateWindowEx(
                  exStyle,
                  pClassName,
                  pWindowName,
                  style,
                  x,
                  y,
                  cx,
                  cy,
                  hwndParent,
                  hmenu,
                  hInstance,
                  this);
            }

            /* Override this method to provide custom message handling. */
            virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
               return DefWindowProc(thisHwnd, msg, wParam, lParam);
            }
      };
   }
}
