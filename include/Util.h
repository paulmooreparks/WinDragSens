#pragma once

namespace SmartSam
{
   namespace Util
   {
      /* Return the number of characters available to be written to in a static 
      array, minus the null terminator. This won't work with a simple pointer; 
      it has to be an array. The entire function call and calculation should 
      optimize away into a single constant number placed at the point of call. */
      template<typename T> inline int const MaxChars(T &x) 
      {
         return size_t(sizeof(x) / sizeof(x[0]) - sizeof(x[0])); 
      }
   }
}
