//#ifdef USE_D16M                      
//      SHORT PrevStrategy;
//
//      PrevStrategy = D16MemStrategy( MTransparent );
//
//      unaddress = D16MemAlloc( 3 );
//
//      D16MemStrategy( PrevStrategy );
//
//#else
//      unaddress = malloc( 3 );
//#endif

      if (unaddress == NULL)
      {
//         error( ERROR_ALLOC_MEM );
         return board_present = FALSE;
      }

      unaddress[0] = 2;
      unaddress[1] = UNL;
      unaddress[2] = UNT;  
   }
