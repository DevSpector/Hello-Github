#include <windows.h>
#include <stdio.h>
#include <string.h>

/* ===== Configuration ===== */
#define WINDOW_WIDTH          350
#define WINDOW_HEIGHT         425
#define BUTTON_WIDTH          60
#define BUTTON_HEIGHT         50
#define DISPLAY_HEIGHT        40
#define DISPLAY_BUFFER_SIZE   50
#define MAX_DIGITS            15

/* ===== Control IDs ===== */
typedef enum {
    ID_BUTTON_0 = 1000,
    ID_BUTTON_1,
    ID_BUTTON_2,
    ID_BUTTON_3,
    ID_BUTTON_4,
    ID_BUTTON_5,
    ID_BUTTON_6,
    ID_BUTTON_7,
    ID_BUTTON_8,
    ID_BUTTON_9,
    ID_BUTTON_ADD,
    ID_BUTTON_SUB,
    ID_BUTTON_MUL,
    ID_BUTTON_DIV,
    ID_BUTTON_EQUALS,
    ID_BUTTON_CLEAR
} ButtonID;

/* ===== Calculator State ===== */
typedef struct {
    double first_operand;
    double second_operand;
    char pending_operation;
    BOOL should_start_new_number;
    BOOL error_state;
} CalculatorState;

/* ===== UI Components ===== */
typedef struct {
    HWND window;
    HWND display;
    HFONT display_font;
} CalculatorUI;

/* ===== Global Context ===== */
static CalculatorUI g_ui;
static CalculatorState g_calc;

/* ===== Function Declarations ===== */
/* Window management */
BOOL initialize_application(HINSTANCE instance, int show_command);
LRESULT CALLBACK main_window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

/* UI management */
BOOL create_calculator_ui(HWND parent_window);
void create_display_field(HWND parent_window);
void create_calculator_buttons(HWND parent_window);
void create_button(HWND parent_window, const char* label, int x, int y, int id);

/* Calculator engine */
void calculator_reset(void);
void calculator_append_digit(int digit);
void calculator_set_operation(char operation);
void calculator_compute_result(void);
void calculator_update_display(const char* text);
void calculator_get_display_text(char* buffer, size_t buffer_size);

/* Event handlers */
void handle_number_button(int digit);
void handle_operation_button(char operation);
void handle_equals_button(void);
void handle_clear_button(void);

/* ===== Implementation ===== */

/* Application entry point */
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int show_command) {
    /* Avoid unused parameter warnings */
    (void)previous_instance;
    (void)command_line;

    if (!initialize_application(instance, show_command)) {
        return 1;
    }

    /* Main message loop */
    MSG message;
    while (GetMessage(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return (int)message.wParam;
}

/* Initializes the application window and resources */
BOOL initialize_application(HINSTANCE instance, int show_command) {
    const char* window_class_name = "CalculatorApp";
    WNDCLASSEX window_class;

    /* Initialize window class structure */
    ZeroMemory(&window_class, sizeof(WNDCLASSEX));
    window_class.cbSize        = sizeof(WNDCLASSEX);
    window_class.style         = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = main_window_procedure;
    window_class.hInstance     = instance;
    window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    window_class.lpszClassName = window_class_name;
    window_class.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    window_class.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&window_class)) {
        MessageBox(NULL, "Window registration failed", "Error", MB_ICONERROR | MB_OK);
        return FALSE;
    }

    g_ui.window = CreateWindowEx(
        0,
        window_class_name,
        "Calculator",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, instance, NULL
    );

    if (g_ui.window == NULL) {
        MessageBox(NULL, "Window creation failed", "Error", MB_ICONERROR | MB_OK);
        return FALSE;
    }

    ShowWindow(g_ui.window, show_command);
    UpdateWindow(g_ui.window);

    return TRUE;
}

/* Main window procedure - handles all window messages */
LRESULT CALLBACK main_window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    switch (message) {
        case WM_CREATE:
            if (!create_calculator_ui(window)) {
                MessageBox(window, "Failed to create UI controls", "Error", MB_ICONERROR | MB_OK);
                return -1;
            }
            calculator_reset();
            break;

        case WM_COMMAND:
            if (HIWORD(w_param) == BN_CLICKED) {
                switch (LOWORD(w_param)) {
                    /* Number buttons */
                    case ID_BUTTON_0: handle_number_button(0); break;
                    case ID_BUTTON_1: handle_number_button(1); break;
                    case ID_BUTTON_2: handle_number_button(2); break;
                    case ID_BUTTON_3: handle_number_button(3); break;
                    case ID_BUTTON_4: handle_number_button(4); break;
                    case ID_BUTTON_5: handle_number_button(5); break;
                    case ID_BUTTON_6: handle_number_button(6); break;
                    case ID_BUTTON_7: handle_number_button(7); break;
                    case ID_BUTTON_8: handle_number_button(8); break;
                    case ID_BUTTON_9: handle_number_button(9); break;

                    /* Operation buttons */
                    case ID_BUTTON_ADD:    handle_operation_button('+'); break;
                    case ID_BUTTON_SUB:    handle_operation_button('-'); break;
                    case ID_BUTTON_MUL:    handle_operation_button('*'); break;
                    case ID_BUTTON_DIV:    handle_operation_button('/'); break;
                    case ID_BUTTON_EQUALS: handle_equals_button(); break;
                    case ID_BUTTON_CLEAR:  handle_clear_button(); break;
                }
            }
            break;

        case WM_DESTROY:
            if (g_ui.display_font) {
                DeleteObject(g_ui.display_font);
            }
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(window, message, w_param, l_param);
    }

    return 0;
}

/* Creates all calculator UI components */
BOOL create_calculator_ui(HWND parent_window) {
    g_ui.window = parent_window;
    
    create_display_field(parent_window);
    create_calculator_buttons(parent_window);
    
    return (g_ui.display != NULL);
}

/* Creates the calculator display field */
void create_display_field(HWND parent_window) {
    g_ui.display = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "0",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_RIGHT | ES_READONLY,
        15, 15,
        WINDOW_WIDTH - 45, DISPLAY_HEIGHT,
        parent_window,
        NULL,
        NULL,
        NULL
    );

    /* Create and set display font */
    g_ui.display_font = CreateFont(
        24, 0, 0, 0, FW_NORMAL, 
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial"
    );
    
    if (g_ui.display_font) {
        SendMessage(g_ui.display, WM_SETFONT, (WPARAM)g_ui.display_font, TRUE);
    }
}

/* Creates all calculator buttons in a grid layout */
void create_calculator_buttons(HWND parent_window) {
    /* Button configuration array */
    struct ButtonConfig {
        const char* label;
        int x;
        int y;
        ButtonID id;
    } buttons[] = {
        {"7", 20, 70, ID_BUTTON_7}, 
        {"8", 90, 70, ID_BUTTON_8}, 
        {"9", 160, 70, ID_BUTTON_9}, 
        {"/", 230, 70, ID_BUTTON_DIV},
        {"4", 20, 130, ID_BUTTON_4}, 
        {"5", 90, 130, ID_BUTTON_5}, 
        {"6", 160, 130, ID_BUTTON_6}, 
        {"*", 230, 130, ID_BUTTON_MUL},
        {"1", 20, 190, ID_BUTTON_1}, 
        {"2", 90, 190, ID_BUTTON_2}, 
        {"3", 160, 190, ID_BUTTON_3}, 
        {"-", 230, 190, ID_BUTTON_SUB},
        {"0", 20, 250, ID_BUTTON_0}, 
        {"C", 90, 250, ID_BUTTON_CLEAR}, 
        {"=", 160, 250, ID_BUTTON_EQUALS}, 
        {"+", 230, 250, ID_BUTTON_ADD}
    };

    /* Create all buttons */
    int i;
    int button_count = sizeof(buttons) / sizeof(buttons[0]);
    
    for (i = 0; i < button_count; i++) {
        create_button(
            parent_window, 
            buttons[i].label, 
            buttons[i].x, 
            buttons[i].y, 
            buttons[i].id
        );
    }
}

/* Creates an individual button */
void create_button(HWND parent_window, const char* label, int x, int y, int id) {
    CreateWindow(
        "BUTTON",
        label,
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        x, y, BUTTON_WIDTH, BUTTON_HEIGHT,
        parent_window,
        (HMENU)(INT_PTR)id,
        NULL,
        NULL
    );
}

/* ===== Calculator Engine ===== */

/* Resets the calculator to its initial state */
void calculator_reset(void) {
    g_calc.first_operand = 0.0;
    g_calc.second_operand = 0.0;
    g_calc.pending_operation = '\0';
    g_calc.should_start_new_number = TRUE;
    g_calc.error_state = FALSE;
    calculator_update_display("0");
}

/* Appends a digit to the current display value */
void calculator_append_digit(int digit) {
    if (g_calc.error_state) {
        return;
    }

    char current_text[DISPLAY_BUFFER_SIZE];
    calculator_get_display_text(current_text, sizeof(current_text));

    if (g_calc.should_start_new_number || strcmp(current_text, "0") == 0) {
        char digit_str[2];
        digit_str[0] = (char)('0' + digit);
        digit_str[1] = '\0';
        calculator_update_display(digit_str);
        g_calc.should_start_new_number = FALSE;
    } else {
        if (strlen(current_text) < MAX_DIGITS) {
            char new_text[DISPLAY_BUFFER_SIZE];
            sprintf(new_text, "%s%d", current_text, digit);
            calculator_update_display(new_text);
        }
    }
}

/* Sets a pending operation for calculation */
void calculator_set_operation(char operation) {
    if (g_calc.error_state) {
        return;
    }

    char display_text[DISPLAY_BUFFER_SIZE];
    calculator_get_display_text(display_text, sizeof(display_text));
    
    g_calc.first_operand = atof(display_text);
    g_calc.pending_operation = operation;
    g_calc.should_start_new_number = TRUE;
}

/* Computes and displays the result of the pending operation */
void calculator_compute_result(void) {
    if (g_calc.error_state || g_calc.pending_operation == '\0') {
        return;
    }

    char display_text[DISPLAY_BUFFER_SIZE];
    calculator_get_display_text(display_text, sizeof(display_text));
    g_calc.second_operand = atof(display_text);

    double result = 0.0;
    BOOL calculation_error = FALSE;

    switch (g_calc.pending_operation) {
        case '+': 
            result = g_calc.first_operand + g_calc.second_operand; 
            break;
        case '-': 
            result = g_calc.first_operand - g_calc.second_operand; 
            break;
        case '*': 
            result = g_calc.first_operand * g_calc.second_operand; 
            break;
        case '/': 
            if (g_calc.second_operand != 0.0) {
                result = g_calc.first_operand / g_calc.second_operand;
            } else {
                calculation_error = TRUE;
            }
            break;
        default:
            /* No operation pending, just show current number */
            result = g_calc.second_operand;
            break;
    }

    if (calculation_error) {
        calculator_update_display("Error: Div by 0");
        g_calc.error_state = TRUE;
    } else {
        char result_text[DISPLAY_BUFFER_SIZE];
        
        /* Format result: use integer format for whole numbers, otherwise 2 decimal places */
        if (result == (int)result) {
            sprintf(result_text, "%d", (int)result);
        } else {
            sprintf(result_text, "%.2f", result);
        }
        
        calculator_update_display(result_text);
        g_calc.first_operand = result;
        g_calc.should_start_new_number = TRUE;
    }
}

/* Updates the calculator display with new text */
void calculator_update_display(const char* text) {
    if (g_ui.display) {
        SetWindowText(g_ui.display, text);
    }
}

/* Retrieves the current display text */
void calculator_get_display_text(char* buffer, size_t buffer_size) {
    if (g_ui.display && buffer) {
        GetWindowText(g_ui.display, buffer, (int)buffer_size);
    }
}

/* ===== Event Handlers ===== */

void handle_number_button(int digit) {
    calculator_append_digit(digit);
}

void handle_operation_button(char operation) {
    calculator_set_operation(operation);
}

void handle_equals_button(void) {
    calculator_compute_result();
}

void handle_clear_button(void) {
    calculator_reset();
}
