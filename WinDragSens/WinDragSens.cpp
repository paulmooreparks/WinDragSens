#include "stdafx.h"
#include "WinDragSens.h"

using namespace ParksComputing::WinDragSens;
using namespace SmartSam::Util;

LPCWSTR App::SETTINGS_PATH = L"ParksComputing";
LPCWSTR App::SETTINGS_FILE = L"WinDragSensProjectSettings.txt";


App::App(HINSTANCE hInstanceInit, LPSTR lpCmdLine, int nShowCmd) :
   isDragging(false),
   SmartSam::AppKit::WinApp(hInstanceInit, lpCmdLine, nShowCmd) 
{
}


App::~App() 
{
}


BOOL App::SetDragWidth(UINT width) 
{
   BOOL success = SystemParametersInfo(SPI_SETDRAGWIDTH, width, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

   if (!success) {
      DWORD error = GetLastError();
      std::wstringstream out;
      out << L"Error " << std::hex << error << std::dec << " while setting drag width.";
      WCHAR caption[100] = {};
      LoadString(GetModuleHandle(NULL), IDS_ERROR_CAPTION, caption, MaxChars(caption));
      MessageBox(NULL, out.str().c_str(), caption, MB_OK);
      return false;
   }

   return success;
}


BOOL App::SetDragHeight(UINT height)
{
   BOOL success = SystemParametersInfo(SPI_SETDRAGHEIGHT, height, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

   if (!success) {
      DWORD error = GetLastError();
      std::wstringstream out;
      out << L"Error " << std::hex << error << std::dec << " while setting drag height.";
      WCHAR caption[100] = {};
      LoadString(GetModuleHandle(NULL), IDS_ERROR_CAPTION, caption, MaxChars(caption));
      MessageBox(NULL, out.str().c_str(), caption, MB_OK);
      return false;
   }

   return success;
}


/* Make any necessary changes to the window class structure prior to registration. */
void App::PreRegisterClass(WNDCLASSEX& wc)
{
   WinApp::PreRegisterClass(wc);

   wc.lpfnWndProc = (WNDPROC)SmartSam::AppKit::Window::BaseDlgProc;
   wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
   wc.hIcon = LoadIcon(GetInstance(), (LPCTSTR)IDI_APP_ICON);
   wc.hIconSm = LoadIcon(GetInstance(), (LPCTSTR)IDI_APP_ICON_SMALL);
   wc.lpszMenuName = NULL;
   wc.cbWndExtra = DLGWINDOWEXTRA;
}


/* All the application startup goes here. This is called from the beginning
of the Run method. */
bool App::Initialize() 
{
   bool retVal = WinApp::Initialize();

   if (retVal) {
      LoadAppSettings();

      /* If LoadAppSettings read valid values from the command line... */
      if (setX || setY) {
         BOOL success = FALSE;

         if (setX) {
            SetDragWidth(xPixels);
         }

         if (setY) {
            SetDragHeight(yPixels);
         }

         /* Don't display the window. Uninitialize and exit. */
         retVal = false;
         goto exitinit;
      }

      INITCOMMONCONTROLSEX iex = {
         sizeof(INITCOMMONCONTROLSEX),
         ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES | ICC_USEREX_CLASSES | ICC_LINK_CLASS
      };

      InitCommonControlsEx(&iex);

      /* Text looks a lot better with smoothing turned on... */
      SystemParametersInfo(SPI_SETFONTSMOOTHING, 1, 0, 0);

      /* ...and better still with ClearType turned on. */
      DWORD param = FE_FONTSMOOTHINGCLEARTYPE;
      SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE, 1, &param, 0);

      LoadString(GetInstance(), IDS_APP_TITLE, SmartSam::AppKit::WinApp::title, SmartSam::Util::MaxChars(SmartSam::AppKit::WinApp::title));
      LoadString(GetInstance(), IDS_WINDOW_CLASS, SmartSam::AppKit::WinApp::windowClass, SmartSam::Util::MaxChars(SmartSam::AppKit::WinApp::windowClass));

      /* Register the window class. */
      if (!DoRegisterClass()) {
         DWORD lastError = GetLastError();
         ReportError(IDS_REGISTERCLASS_FAILED);
         retVal = false;
         goto exitinit;
      }

      /* Use a dialog as the main application window. */
      HWND hwnd = CreateDialogParam(GetInstance(), MAKEINTRESOURCE(IDD_APP_DIALOG), 0, SmartSam::AppKit::Window::BaseDlgProc, (LPARAM)this);

      if (!hwnd) {
         ReportError(IDS_CREATEWINDOW_FAILED);
         retVal = false;
         goto exitinit;
      }

      hAccelTable = LoadAccelerators(GetInstance(), (PCWSTR)IDC_APP_ACCEL);

      if (!hAccelTable) {
         ReportError(IDS_LOADACCELERATORS_FAILED);
         retVal = false;
         goto exitinit;
      }

      /* If window placement was loaded from the settings file, move the window. */
      if (settings.isPlacement) {
         SetWindowPlacement(GetHWND(), &settings.wp);

         if (showCmd != SW_SHOWMINIMIZED) {
            showCmd = settings.wp.showCmd;

            if (showCmd == SW_SHOWMINIMIZED) {
               showCmd = SW_RESTORE;
            }

            if (settings.wp.flags == WPF_RESTORETOMAXIMIZED) {
               showCmd = SW_MAXIMIZE;
            }
         }
      }
      /* Otherwise, center the window on the screen. */
      else {
         RECT rect = {};
         GetWindowRect(hwnd, &rect);
         int screenWidth = GetSystemMetrics(SM_CXSCREEN);
         int screenHeight = GetSystemMetrics(SM_CYSCREEN);
         int width = rect.right - rect.left;
         int height = rect.bottom - rect.top;
         int x = (screenWidth / 2) - (width / 2) + rect.left;
         int y = (screenHeight / 2) - (height / 2) + rect.top;
         MoveWindow(hwnd, x, y, width, height, FALSE);
      }

      ShowWindow(hwnd, showCmd);
      UpdateWindow(hwnd);

      retVal = true;
   }

exitinit:
   if (!retVal) {
      Uninitialize();
   }

   return retVal;
}


/* Uninitialize everything that needs to be uninitialized before we end the app.
This is called before the Run method returns. MAKE SURE that this method may be
called without errors even if Initialize did not complete successfully. */
void App::Uninitialize() 
{
   SaveAppSettings();
}


/* Display a message box with an error message from the string table. */
void App::ReportError(UINT id) 
{
   WCHAR caption[100] = {};
   LoadString(GetModuleHandle(NULL), IDS_ERROR_CAPTION, caption, MaxChars(caption));

   WCHAR error[100] = {};
   LoadString(GetModuleHandle(NULL), id, error, MaxChars(error));

   MessageBox(NULL, error, caption, MB_OK);
}


/* Build a path to an app-specific directory in the user's application
data storage directory. */
LPWSTR App::BuildSettingsPath(LPWSTR destPath, LPCWSTR fileName) 
{
   if (destPath) {
      HRESULT hr = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, destPath);

      if (SUCCEEDED(hr)) {
         if (PathCombine(destPath, destPath, SETTINGS_PATH)) {
            BOOL created = CreateDirectory(destPath, NULL);

            if (created || GetLastError() == ERROR_ALREADY_EXISTS) {
               return PathCombine(destPath, destPath, fileName);
            }
         }
      }
   }

   return 0;
}


void App::Usage() 
{
   OnCommand(GetHWND(), IDM_ABOUT, 0, 0);
}


bool App::LoadAppSettings() 
{
   bool success = true;

   LPWSTR cmdLine = GetCommandLine();

   /* Tokenize the command line and save the tokens in a vector. */
   using namespace std;
   wstring textLine = cmdLine;
   wistringstream iss(textLine);
   vector<wstring> args;
   copy(istream_iterator<wstring, wchar_t>(iss), istream_iterator<wstring, wchar_t>(), back_inserter<vector<wstring> >(args));

   setX = false;
   xPixels = 0;
   setY = false;
   yPixels = 0;

   UINT numPixels = 0;
   std::wstringstream cvt;
   vector<wstring>::const_iterator it = args.begin();

   /* Walk through the token vector and parse the parameters. */
   if (args.size() > 1) {
      /* Skip the application name */
      ++it;

      if (cvt << *it && cvt >> numPixels) {
         xPixels = numPixels;
         setX = true;
         yPixels = numPixels;
         setY = true;
      }
      else {
         do {
            cvt.clear();
            cvt.str(L"");

            std::wstring arg = *it;
            wchar_t c = arg[0];

            if (c == L'/' || c == L'-') {
               c = arg[1];

               if (c == L'?') {
                  Usage();
                  return false;
               }
               else if (c == L'x' || c == L'X') {
                  ++it;

                  if (cvt << *it && cvt >> xPixels) {
                     setX = true;
                  }
                  else {
                     Usage();
                     return false;
                  }
               }
               else if (c == L'y' || c == L'Y') {
                  ++it;

                  if (cvt << *it && cvt >> yPixels) {
                     setY = true;
                  }
                  else {
                     Usage();
                     return false;
                  }
               }
               else {
                  Usage();
                  return false;
               }
            }

            ++it;
         } while (it != args.end());
      }
   }

   WCHAR pPath[MAX_PATH] = {};

   /* Read saved settings from the settings file. */
   if (success && BuildSettingsPath(pPath, SETTINGS_FILE)) {
      HANDLE hFile = CreateFile(pPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

      if (hFile != INVALID_HANDLE_VALUE) {
         /* Go to end and get file size. */
         DWORD fileSize = SetFilePointer(hFile, 0, NULL, FILE_END);
         SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

         DWORD bytesRead = 0;
         std::vector<CHAR> buf(fileSize + 1);
         ReadFile(hFile, &buf[0], fileSize, &bytesRead, 0);

         std::string payload = &buf[0];

         std::stringstream cvt;
         cvt << &buf[0];
         cvt >> settings;

         success = !!CloseHandle(hFile);
      }
   }

   return success;
}


bool App::SaveAppSettings() 
{
   bool success = false;
   WCHAR pPath[MAX_PATH] = {};

   if (BuildSettingsPath(pPath, SETTINGS_FILE)) {
      HANDLE hFile = CreateFile(
         pPath,
         GENERIC_READ | GENERIC_WRITE,
         FILE_SHARE_READ | FILE_SHARE_WRITE,
         NULL,
         CREATE_ALWAYS,
         FILE_ATTRIBUTE_NORMAL,
         0);

      if (hFile != INVALID_HANDLE_VALUE) {
         DWORD bytesWritten = 0;
         std::stringstream cvt;
         cvt << settings;
         std::string settingStr;
         std::getline(cvt, settingStr);
         std::vector<char> buf(settingStr.begin(), settingStr.end());
         WriteFile(hFile, &buf[0], buf.size(), &bytesWritten, NULL);
         success = !!CloseHandle(hFile);
      }
   }

   return success;
}


INT_PTR CALLBACK App::AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
   static App* pApp = 0;

   if (msg == WM_INITDIALOG) {
      pApp = (App*)lParam;
   }

   if (pApp) {
      return pApp->About(hDlg, msg, wParam, lParam);
   }

   return (INT_PTR)FALSE;
}


/* Display the about box. */
INT_PTR App::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/) 
{
   switch (message) {
      case WM_INITDIALOG: {
         WCHAR helpText[4097] = {};
         LoadString(GetInstance(), IDS_HELPTEXT, helpText, MaxChars(helpText));
         Edit_SetText(GetDlgItem(hDlg, IDC_HELPTEXT), /*App::HELPTEXT*/ helpText);
      }
      return (INT_PTR)TRUE;

      case WM_COMMAND:
         if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
         }
         break;
   }

   return (INT_PTR)FALSE;
}


BOOL App::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
   /* Initialize the list view used as a drag test. */
   HWND hListView = GetDlgItem(hwnd, IDC_DRAG_TEST);
   HIMAGELIST hLarge = ImageList_Create(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), ILC_MASK, 1, 1);
   HICON hIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_APP_ICON));
   ImageList_AddIcon(hLarge, hIcon);
   DestroyIcon(hIcon);
   ListView_SetImageList(hListView, hLarge, LVSIL_NORMAL);

   LVITEM lvI = {};

   // Initialize LVITEM members that are common to all items.
   lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
   lvI.stateMask = 0;
   lvI.iSubItem = 0;
   lvI.state = 0;

   lvI.pszText = L"Drag Me!";
   lvI.iItem = 0;
   lvI.iImage = 0;

   ListView_InsertItem(hListView, &lvI);

   /* Initialize the edit controls with the current drag settings. */
   int xPixels = GetSystemMetrics(SM_CXDRAG);
   int yPixels = GetSystemMetrics(SM_CYDRAG);

   std::wstringstream cvt;
   cvt << xPixels;
   std::wstring out;
   cvt >> out;

   Edit_SetText(GetDlgItem(hwnd, IDC_HORIZONTAL), out.c_str());

   cvt.clear();
   cvt.str(L"");
   cvt << yPixels;
   cvt >> out;

   Edit_SetText(GetDlgItem(hwnd, IDC_VERTICAL), out.c_str());

   return TRUE;
}


LRESULT App::OnNotify(HWND hwnd, int id, NMHDR* pnmHdr) 
{
   switch (pnmHdr->code) {
      case LVN_BEGINDRAG:
         POINT pt = { 4, 4 };
         HWND hListView = GetDlgItem(hwnd, IDC_DRAG_TEST);
         int pos = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
         hDragImageList = ListView_CreateDragImage(hListView, pos, &pt);
         ImageList_BeginDrag(hDragImageList, 0, 0, 0);
         pt = ((NMLISTVIEW*)pnmHdr)->ptAction;
         ClientToScreen(hListView, &pt);
         ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);
         isDragging = true;
         SetCapture(GetHWND());
         break;
   }
   return 0;
}


void App::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags) 
{
   if (isDragging) {
      POINT pt = {x, y};
      ClientToScreen(hwnd, &pt);
      ImageList_DragMove(pt.x, pt.y);
   }
}


void App::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags) 
{
   isDragging = false;
   HWND hListView = GetDlgItem(hwnd, IDC_DRAG_TEST);
   ImageList_DragLeave(hListView);
   ImageList_EndDrag();
   ImageList_Destroy(hDragImageList);
   ReleaseCapture();
}


/* Post a quit message because our application is over when the user closes this window. */
void App::OnDestroy(HWND hwnd) 
{
   GetWindowPlacement(GetHWND(), &settings.wp);
   PostQuitMessage(0);
}


BOOL App::OnQueryEndSession(HWND hwnd) 
{
   /* Only return FALSE here if shutting down the app would make the user cry. */
   return TRUE;
}


void App::OnEndSession(HWND hwnd, BOOL ending) 
{
   SaveAppSettings();
}


void App::OnClose(HWND hwnd) 
{
   DestroyWindow(hwnd);
}


void App::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) 
{
   switch (id) {
      case IDM_ABOUT:
         DialogBoxParam(GetInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, AboutDlgProc, (LPARAM)this);
         break;

      case IDC_SET:
      {
         /* The set button was activated, so save the settings in the edit controls. */
         HWND hHoriz = GetDlgItem(hwnd, IDC_HORIZONTAL);
         HWND hVert = GetDlgItem(hwnd, IDC_VERTICAL);

         WCHAR horizontal[20] = {};
         Edit_GetText(hHoriz, horizontal, MaxChars(horizontal));

         WCHAR vertical[20] = {};
         Edit_GetText(hVert, vertical, MaxChars(vertical));

         std::wstringstream cvt;

         int xPixels;
         cvt << horizontal;
         cvt >> xPixels;
         SetDragWidth(xPixels);

         cvt.clear();
         cvt.str(L"");

         int yPixels;
         cvt << vertical;
         cvt >> yPixels;
         SetDragHeight(yPixels);

         break;
      }

      case IDM_EXIT:
         DestroyWindow(hwnd);
         break;
   }
}


/* Window procedure */
LRESULT App::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) 
{
   switch (msg) {
      HANDLE_MSG(GetHWND(), WM_INITDIALOG, OnInitDialog);
      HANDLE_MSG(GetHWND(), WM_DESTROY, OnDestroy);
      HANDLE_MSG(GetHWND(), WM_COMMAND, OnCommand);
      HANDLE_MSG(GetHWND(), WM_NOTIFY, OnNotify);
      HANDLE_MSG(GetHWND(), WM_QUERYENDSESSION, OnQueryEndSession);
      HANDLE_MSG(GetHWND(), WM_ENDSESSION, OnEndSession);
      HANDLE_MSG(GetHWND(), WM_CLOSE, OnClose);
      HANDLE_MSG(GetHWND(), WM_MOUSEMOVE, OnMouseMove);
      HANDLE_MSG(GetHWND(), WM_LBUTTONUP, OnLButtonUp);
   }

   return WinApp::WndProc(msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR pCmdLine, int showCmd) 
{
   ParksComputing::WinDragSens::App app(hInstance, pCmdLine, showCmd);
   return app.Run();
}

