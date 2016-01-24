#pragma once

#include "resource.h"
#include "WinApp.h"
#include "Util.h"
#include <queue>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>

#include <ShlObj.h>
#include <Shlwapi.h>

namespace ParksComputing {
   namespace WinDragSens {

      struct Settings {
         public:
            bool isPlacement;
            WINDOWPLACEMENT wp;

            Settings() : isPlacement(false) {
               wp.length = sizeof(WINDOWPLACEMENT);
            }

            friend std::istream& operator>> (std::istream& s, Settings& obj);
            friend std::ostream& operator<< (std::ostream& o, Settings const& b);

         protected:
            virtual void DoOutput(std::ostream& s) const {
               s << wp.flags << " ";
               s << wp.ptMaxPosition.x << " " << wp.ptMaxPosition.y << " ";
               s << wp.ptMinPosition.x << " " << wp.ptMinPosition.y << " ";
               s << wp.rcNormalPosition.left << " " << wp.rcNormalPosition.top << " " << wp.rcNormalPosition.right << " " << wp.rcNormalPosition.bottom << " ";
               s << wp.showCmd;
            }

            virtual void DoInput(std::istream& s) {
               isPlacement = false;

               if (s >> wp.flags
                  >> wp.ptMaxPosition.x >> wp.ptMaxPosition.y
                  >> wp.ptMinPosition.x >> wp.ptMinPosition.y
                  >> wp.rcNormalPosition.left >> wp.rcNormalPosition.top >> wp.rcNormalPosition.right >> wp.rcNormalPosition.bottom
                  >> wp.showCmd) {
                  isPlacement = true;
               }
            }

      };

      std::ostream& operator<< (std::ostream& s, Settings const& obj) {
         obj.DoOutput(s);
         return s;
      }

      std::istream& operator>>(std::istream& s, Settings& obj) {
         obj.DoInput(s);
         return s;
      }

      class App : public SmartSam::AppKit::WinApp {
         public:
            App(HINSTANCE hInstanceInit, LPSTR lpCmdLine, int nShowCmd);
            virtual ~App();

         protected:
            bool setX;
            UINT xPixels;
            bool setY;
            UINT yPixels;
            bool isDragging;
            HIMAGELIST hDragImageList;
            Settings settings;

         protected:
            bool Initialize();
            void Uninitialize();
            void PreRegisterClass(WNDCLASSEX& wc);
            LPWSTR BuildSettingsPath(LPWSTR destPath, LPCWSTR fileName);
            LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
            static INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
            INT_PTR About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
            BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
            void OnDestroy(HWND hwnd);
            LRESULT OnNotify(HWND hwnd, int id, NMHDR* pnmHdr);
            void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
            void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
            void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
            BOOL OnQueryEndSession(HWND hwnd);
            void OnEndSession(HWND hwnd, BOOL fEnding);
            void OnClose(HWND hwnd);
            void ReportError(UINT id);
            bool LoadAppSettings();
            bool SaveAppSettings();
            void Usage();

            BOOL SetDragWidth(UINT width);
            BOOL SetDragHeight(UINT height);

            static LPCWSTR SETTINGS_PATH;
            static LPCWSTR SETTINGS_FILE;
            static LPCWSTR PARKSCOMPUTING_REG_BASE;
            static LPCWSTR APP_REG_BASE;
      };
   }
}

