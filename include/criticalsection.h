/****************************************************************************
                       
Author: 
   Paul M. Parks (pp230000) 
   paul@parkscomputing.com

*****************************************************************************/

#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H


#include <windows.h>
#include <string>
#include <exception>

/*

  Usage:
   This class makes it easier to serialize access to data that 
   is manipulated by multiple threads. Create a lock at the 
   start of a block, and it will be automatically released when 
   the lock goes out of scope.

  Example:

   #include <CriticalSection.h>

   using namespace ParksComputing::Util;

   // Creates the critical section instance. Multiple instances 
   // may be created, as necessary.
   CriticalSection cs;

   // This data is accessed by multiple threads.
   int foo = 0;

   DWORD Thread(void* param)
   {
      // Acquire a lock
      Lock lock(cs);
      foo = foo + 2;
   }
   // Lock is automatically released here.

   int main()
   {
      HANDLE h[2];

      // First thread.
      h[0] = (HANDLE)::_beginthreadex(
         NULL, 
         0, 
         reinterpret_cast<unsigned int(__stdcall *)(void*)>(Thread), 
         NULL, 
         0, 
         NULL
         );

      // Second thread.
      h[1] = (HANDLE)::_beginthreadex(
         NULL, 
         0, 
         reinterpret_cast<unsigned int(__stdcall *)(void*)>(Thread), 
         NULL, 
         0, 
         NULL
         );

      WaitForMultipleObjects(2, h, TRUE, INFINITE);
    }

*/

namespace ParksComputing
{
   namespace Util
   {
      class CriticalSection
      {
      public:
         CriticalSection()
         {
            ::InitializeCriticalSection(&cs_);
         }
         
         CriticalSection(CRITICAL_SECTION cs) : cs_(cs)
         {
         }
         
         ~CriticalSection()
         {
            ::DeleteCriticalSection(&cs_);
         }
         
         void Enter()
         {
            ::EnterCriticalSection(&cs_);
         }
         
         void Leave()
         {
            ::LeaveCriticalSection(&cs_);
         }
         
      private:
         CRITICAL_SECTION cs_;
         
         CRITICAL_SECTION& operator=(CRITICAL_SECTION& cs);
      };
      
      
      class Lock
      {
      public:
         Lock(CriticalSection& cs) : cs_(cs)
         {
            cs_.Enter();
         }
         
         ~Lock()
         {
            cs_.Leave();
         }
         
      private:
         CriticalSection& cs_;
         
         // A developer is not allowed to create a standalone Lock.
         Lock();
      };
      
      
      class MutexObject
      {
      public:
         typedef HANDLE SynchType;
         
         MutexObject(
            LPSECURITY_ATTRIBUTES psa = NULL,
            bool bInitOwner = false,
            LPCTSTR pname = NULL,
            DWORD dwDesiredAccess = MUTEX_ALL_ACCESS,
            bool bInheritable = false
            ) : psa_(psa), bInitOwner_(bInitOwner), dwDesiredAccess_(dwDesiredAccess), bInheritable_(bInheritable)
         {
            if (pname)
            {
               name_ = pname;
            }

            h_ = ::CreateMutex(
               psa_, 
               bInitOwner_,
               pname ? name_.c_str() : 0
               );
         }

         ~MutexObject()
         {
            ::CloseHandle(h_);
         }
         
         DWORD Acquire(DWORD timeoutMillis)
         {
            return ::WaitForSingleObject(
               h_,
               timeoutMillis
               );
         }
         
         bool TryAcquire()
         {
            DWORD waitStatus = ::WaitForSingleObject(
               h_,
               0
               );
            
            if (waitStatus == WAIT_OBJECT_0)
            {
               return true;
            }
            
            return false;
         }
         
         void Release()
         {
            ::ReleaseMutex(h_);
         }
         
         static DWORD Acquire(
            HANDLE* sa, 
            DWORD count, 
            bool waitAll,
            DWORD timeoutMillis,
            bool alertable
            )
         {
            return ::WaitForMultipleObjectsEx(count, sa, waitAll, timeoutMillis, alertable);
         }
         
         static void Release(HANDLE* sa, size_t count)
         {
            while (count > 0)
            {
               --count;
               ::ReleaseMutex(sa[count]);
            }
         }
         
         
         HANDLE h_;
         
      private:
         LPSECURITY_ATTRIBUTES psa_;
         bool bInitOwner_;
         std::wstring name_;
         DWORD dwDesiredAccess_;
         bool bInheritable_;
      };


      class acquire_timeout_error : public std::runtime_error 
      {
      public:
         explicit acquire_timeout_error(const std::string& s) : runtime_error(s) 
         {
         }

         virtual ~acquire_timeout_error()
         {
         }
      };


      class acquire_abandoned_error : public std::runtime_error 
      {
      public:
         explicit acquire_abandoned_error(const std::string& s) : runtime_error(s) 
         {
         }

         virtual ~acquire_abandoned_error()
         {
         }
      };

      
      class Mutex
      {
      public:
         Mutex(MutexObject& mo, DWORD timeoutMillis = 5000) : mo_(mo)
         {
            DWORD result = mo_.Acquire(timeoutMillis);

            switch (result)
            {
            case WAIT_OBJECT_0:
               break;

            case WAIT_TIMEOUT:
               throw acquire_timeout_error("Mutex acquisition timed out");
               break;

            case WAIT_ABANDONED:
               throw acquire_abandoned_error("Acquired an abandoned mutex object");
               break;

            default:
               throw std::runtime_error("Unknown status from mutex acquisition");
               break;
            }
         }
         
         ~Mutex()
         {
            mo_.Release();
         }
         
      private:
         MutexObject& mo_;
         
         // A developer is not allowed to create a standalone Lock.
         Mutex();
      };
      
      
   }
}

#endif // #ifndef CRITICALSECTION_H
