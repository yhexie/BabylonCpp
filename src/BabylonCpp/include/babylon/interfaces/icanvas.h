#ifndef BABYLON_INTERFACES_ICANVAS_H
#define BABYLON_INTERFACES_ICANVAS_H

#include <babylon/babylon_global.h>
#include <babylon/core/fast_func.h>
#include <babylon/core/structs.h>
#include <babylon/engine/engine_options.h>
#include <babylon/interfaces/igl_rendering_context.h>

namespace BABYLON {

class BABYLON_SHARED_EXPORT ICanvas {

public:
  ICanvas();
  virtual ~ICanvas();
  virtual ClientRect& getBoundingClientRect()       = 0;
  virtual bool onlyRenderBoundingClientRect() const = 0;
  virtual bool initializeContext3d()                = 0;
  virtual ICanvasRenderingContext2D* getContext2d() = 0;
  virtual GL::IGLRenderingContext* getContext3d(const EngineOptions& options)
    = 0;
  // Event listeners
  void addMouseEventListener(EventType type,
                             const FastFunc<void(PointerEvent&& evt)>& listener,
                             bool useCapture = false);
  void addKeyEventListener(EventType type,
                           const FastFunc<void(Event&& evt)>& listener,
                           bool useCapture = false);
  void
  removeMouseEventListener(EventType type,
                           const FastFunc<void(PointerEvent&& evt)>& listener);
  void removeKeyEventListener(EventType type,
                              const FastFunc<void(Event&& evt)>& listener);

  void setFrameSize(int width, int height);
  void onError(int errorId, const char* errorDesc);
  // User unput handling
  void onMouseButtonDown(int mouseX, int mouseY, MouseButtonType btn);
  void onMouseButtonUp(int mouseX, int mouseY, MouseButtonType btn);
  void onMouseMove(int mouseX, int mouseY);
  void onMouseWheel(float wheelDelta);
  void onKeyDown(bool ctrlKey, int keyCode);
  void onKeyUp(bool ctrlKey, int keyCode);

  void focus();
  void applicationDidEnterBackground();
  void applicationWillEnterForeground();
  void setPointerCapture(int pointerId);
  void releasePointerCapture(int pointerId);

public:
  int width;
  int height;
  int clientWidth;
  int clientHeight;
  int mouseX;
  int mouseY;
  int tabIndex;
  Style style;
  ClientRect _boundingClientRect;

protected:
  std::unique_ptr<GL::IGLRenderingContext> _renderingContext;
  std::vector<std::vector<FastFunc<void(PointerEvent&& evt)>>>
    mouseEventListeners;
  std::vector<std::vector<FastFunc<void(Event&& evt)>>> keyEventListeners;

protected:
  bool _initialized;

}; // end of class ICanvas

} // end of namespace BABYLON

#endif // end of BABYLON_INTERFACES_ICANVAS_H
