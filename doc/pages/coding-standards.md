Coding Standards
================

The VUEngine and any official demo code is being written adhering to the following coding standards to ensure a 
consistent code appearance and high interoperability. 

- Tabs of size 4 must be used for indenting.
- Method names must be declared in StudlyCaps and follow the scheme [ClassName]_[MethodName].
- The first argument of every method must be a pointer to an instance of the respective object called "this".
- The first line of code of each method should be an ASSERT that verifies that the "this" pointer is not NULL.
- Macros must be declared in all upper case with underscore separators. Engine macros additionally must be prefixed with two underscores.
- Opening braces for methods must go on the next line, and closing braces must go on the next line after the body.
- Opening braces for control structures must go on the next line, and closing braces must go on the next line after the body.
- The soft limit for line length is 107 characters.
