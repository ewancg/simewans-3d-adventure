#pragma once
#include "Window.h"
#include <bitset>

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing input")                                                                    \
  E(EVENT, "processing input events")                                                              \
  E(SET_MOUSE_POS, "setting the mouse cursor position")
DEFINE_DERIVED_ERROR_TYPES(Input, Subsystem, ERROR_ENTRIES);
#undef ERROR_ENTRIES

enum EInputMapping : uint8_t {
  KEY_A = 0,
  KEY_B,
  KEY_C,
  KEY_D,
  KEY_E,
  KEY_F,
  KEY_G,
  KEY_H,
  KEY_I,
  KEY_J,
  KEY_K,
  KEY_L,
  KEY_M,
  KEY_N,
  KEY_O,
  KEY_P,
  KEY_Q,
  KEY_R,
  KEY_S,
  KEY_T,
  KEY_U,
  KEY_V,
  KEY_W,
  KEY_X,
  KEY_Y,
  KEY_Z,

  KEY_0, // NOLINT(misc-confusable-identifiers)
  KEY_1, // NOLINT(misc-confusable-identifiers)
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,

  KEY_RETURN,
  KEY_SPACE,
  KEY_ESCAPE,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_UP,
  KEY_DOWN,

  MOD_LSHIFT,
  MOD_LCTRL,
  MOD_LALT,
  MOD_RSHIFT,
  MOD_RCTRL,
  MOD_RALT,

  MOUSE_LEFT,
  MOUSE_RIGHT,
  MOUSE_MIDDLE,
  MOUSE_WHEEL_DOWN,
  MOUSE_WHEEL_UP,
  MOUSE_X1,
  MOUSE_X2,

  LENGTH
};

class Input : public Subsystem<InputError> {
  SUBSYSTEM(Input)
  Application &m_app;
  Window &m_window;

public:
  NO_COPY_MOVE_OR_ASSIGN(Input, "", "")
  explicit Input(Application &t_app, Window &t_window) : m_app(t_app), m_window(t_window) {}
  ~Input() = default;
  /// Checks if the input is currently down
  bool mappingIsPressed(EInputMapping t_in);
  /// Checks if the input is currently down, and was not last frame
  bool mappingNewlyPressed(EInputMapping t_in);
  /// Sets the mouse cursor to an absolute position
  Error setMousePos(uint32_t t_x, uint32_t t_y);
  /// Centers the cursor within the screen
  Error centerMousePos();

private:
  struct MouseData {
    float x, y;
    float xm, ym;
  } m_mouseData{};
  std::bitset<EInputMapping::LENGTH> m_inputState{0};
  std::bitset<EInputMapping::LENGTH> m_lastInputState{0};

  static bool getKeyState(const SDL_KeyboardEvent &t_evt);
  ApplicationError keyboardEvent(Event &t_evt);
  ApplicationError mouseButtonEvent(Event &t_evt);
  ApplicationError mouseMotionEvent(Event &t_evt);
};

// clang-format off
#define INPUT_CASE(PREFIX, SDL_PREFIX, STATE, RHS, NAME)   \
  case SDL_PREFIX##_##NAME:                                \
    (STATE)[PREFIX##_##NAME] = (RHS);                      \
    break;

#define KB_INPUT_KEY_CASES(FN) \
  FN(A) FN(B) FN(C) FN(D) FN(E) FN(F) FN(G) FN(H) FN(I) FN(J) FN(K) FN(L) FN(M) \
  FN(N) FN(O) FN(P) FN(Q) FN(R) FN(S) FN(T) FN(U) FN(V) FN(W) FN(X) FN(Y) FN(Z) \
  FN(0) FN(1) FN(2) FN(3) FN(4) FN(5) FN(6) FN(7) FN(8) FN(9)                \
  FN(RETURN) FN(SPACE) FN(ESCAPE) FN(LEFT) FN(RIGHT) FN(UP) FN(DOWN)

#define KB_INPUT_MODIFIER_CASES(FN) \
  FN(LSHIFT) FN(LCTRL) FN(LALT) FN(RSHIFT) FN(RCTRL) FN(RALT)

#define MOUSE_INPUT_BUTTON_CASES(FN) \
  FN(LEFT) FN(MIDDLE) FN(RIGHT) FN(X1) FN(X2)
// clang-format on
