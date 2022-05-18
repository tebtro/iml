#if !defined(IML_TERMINAL_H)
#define IML_TERMINAL_H



bool terminal_ansi_colors_supported(void) {
#if OS_WINDOWS
    HANDLE handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle_stdout != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(handle_stdout, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            if (SetConsoleMode(handle_stdout, mode)) {
                return true;
            }
        }
    }
#elif OS_LINUX || OS_MAC
    const char *term = getenv("TERM");
    return term && (strcmp(term, "") != 0);
#else
#  error "OS not implemented!"
#endif
    
    return false;
}


#define COLOR_RESET    "\x1B[0m"
#define COLOR_BOLD     "\x1B[1m"
#define COLOR_RED      "\x1B[31m"
#define COLOR_GREEN    "\x1B[32m"
#define COLOR_MAGENTA  "\x1B[35m"


struct Terminal_Colors {
    const char *reset;
    const char *bold;
    const char *red;
    const char *green;
    const char *magenta;
};

Terminal_Colors get_terminal_colors(void) {
    Terminal_Colors colors;
    
    bool ansi_colors_supported = terminal_ansi_colors_supported();
    
    if (ansi_colors_supported) {
        colors.reset   = COLOR_RESET;
        colors.bold    = COLOR_BOLD;
        colors.red     = COLOR_RED;
        colors.green   = COLOR_GREEN;
        colors.magenta = COLOR_MAGENTA;
    }
    else {
        colors.reset   = "";
        colors.bold    = "";
        colors.red     = "";
        colors.green   = "";
        colors.magenta = "";
    }
    
    return colors;
}



#endif // IML_TERMINAL_H
