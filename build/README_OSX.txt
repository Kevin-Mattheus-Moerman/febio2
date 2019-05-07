On Mac, use this command to allow the febio executable to run when libiomp5.dylib is located in the same folder as the executable:

install_name_tool -change @rpath/libiomp5.dylib @executable_path/libiomp5.dylib febio2.osx

