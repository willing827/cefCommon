#pragma once
#include "include/cef_app.h"
#include "util/util.h"

/** @class CefManager
 * @brief Cef�����ʼ��������
 */
class CefManager
  :public helper_lib::SingletonT<CefManager>
{
  HELPERLIBSINGLETON(CefManager);
public:
  static bool PostTaskUIThread(std::function<void()> const& task);

  static bool PostTaskUIThread(helper_lib::AsyncTask *task);

  /**
  * ��cef dll�ļ���λ����ӵ������"path"����������,�������԰�dll�ļ��ŵ�bin�����Ŀ¼�����Ҳ���Ҫ�ֶ�Ƶ���л�dll�ļ�
  * @param[in] cef_path cef��,��Դ,ִ���ļ���·��
  * @return void	�޷���ֵ
  */
  void AddCefDllToPath(LPCWSTR cef_path = nullptr);

  /**
  * ��ʼ��cef���
  * @param[in] app_data_dir Ӧ��·������
  * @return bool true �������У�false Ӧ�ý�������
  */
  bool Initialize(const std::wstring& app_data_dir, CefSettings& settings);

  /**
  * ����cef���
  * @return void	�޷���ֵ
  */
  void UnInitialize();

  // ��¼��������������
  void AddBrowserCount();

  void SubBrowserCount();

  int	GetBrowserCount();
private:
  /**
  * ����cef������Ϣ
  * @param[in] app_data_dir Ӧ��·������
  * @param[out] settings cef������
  * @return void	�޷���ֵ
  */
  void GetCefSetting(const std::wstring& app_data_dir, CefSettings& settings);

private:
  CefManager();
  ~CefManager() = default;
private:
  int browser_count_;

  std::unique_ptr<helper_lib::MessageWnd> message_wnd_;
};