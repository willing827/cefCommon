#pragma once
#include "include/cef_app.h"
#include "util/util.h"

/** @class CefManager
 * @brief Cef组件初始化和销毁
 */
class CefManager
  :public helper_lib::SingletonT<CefManager>
{
  HELPERLIBSINGLETON(CefManager);
public:
  static bool PostTaskUIThread(std::function<void()> const& task);

  static bool PostTaskUIThread(helper_lib::AsyncTask *task);

  /**
  * 把cef dll文件的位置添加到程序的"path"环境变量中,这样可以把dll文件放到bin以外的目录，并且不需要手动频繁切换dll文件
  * @param[in] cef_path cef库,资源,执行文件包路劲
  * @return void	无返回值
  */
  void AddCefDllToPath(LPCWSTR cef_path = nullptr);

  /**
  * 初始化cef组件
  * @param[in] app_data_dir 应用路径名称
  * @return bool true 继续运行，false 应该结束程序
  */
  bool Initialize(const std::wstring& app_data_dir, CefSettings& settings);

  /**
  * 清理cef组件
  * @return void	无返回值
  */
  void UnInitialize();

  // 记录浏览器对象的数量
  void AddBrowserCount();

  void SubBrowserCount();

  int	GetBrowserCount();
private:
  /**
  * 设置cef配置信息
  * @param[in] app_data_dir 应用路径名称
  * @param[out] settings cef配置类
  * @return void	无返回值
  */
  void GetCefSetting(const std::wstring& app_data_dir, CefSettings& settings);

private:
  CefManager();
  ~CefManager() = default;
private:
  int browser_count_;

  std::unique_ptr<helper_lib::MessageWnd> message_wnd_;
};