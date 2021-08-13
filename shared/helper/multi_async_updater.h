/*With the exception of:
- FONT AWESOME

THE MATERIAL IS LICENSED BY ME AS

MIT License

Copyright (c) 2016 Jim Credland

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
https://github.com/jcredland/jcf_lime_juce/blob/master/utils/multi_async_updater.h
*/

#pragma once

class MultiAsyncUpdater : AsyncUpdater {
 public:
  MultiAsyncUpdater() {}

  // pass by value is more efficent where the std::function will be created in
  // place from a lambda.  See
  // http://stackoverflow.com/questions/18365532/should-i-pass-an-stdfunction-by-const-reference
  void callOnMessageThread(std::function<void()> callback) {
    ScopedLock l(lock);
    queue.push_back(std::move(callback));
    triggerAsyncUpdate();
  }

 private:
  void handleAsyncUpdate() override {
    lock.enter();
    auto queueCopy = queue;
    queue.clear();
    lock.exit();

    for (auto& func : queueCopy) func();
  }

  CriticalSection lock;
  std::vector<std::function<void()>> queue;
};
