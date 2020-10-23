#pragma once

#include <functional>
#include "include/cef_app.h"

namespace cef_render
{
  typedef std::map<std::pair<CefString/* function_name*/, int64_t /*frame_id*/>, CefRefPtr<CefV8Value>/* function*/> RenderRegisteredFunction;

  class CefJSBridge
  {
  public:
    CefJSBridge();
    ~CefJSBridge();

  public:
    /**
     * ִ���Ѿ�ע��õ� C++ ����
     * param[in] function_name	Ҫ���õĺ�������
     * param[in] params			���øú������ݵ� json ��ʽ����
     * return ���� true ��ʾ����ִ������ɹ�
     */
    bool CallCppFunction(const CefString& function_name, const CefString& params);


    /**
     * ע��һ���־õ� JS �����ṩ C++ ����
     * param[in] function_name	�������ƣ��ַ�����ʽ�ṩ C++ ֱ�ӵ��ã����Ʋ����ظ�
     * param[in] func		������
     */
    void RegisterJSFunc(const CefString& function_name, CefRefPtr<CefV8Value> func, int64_t frame_id);

    /**
     * ��ע��һ���־õ� JS ����
     * param[in] function_name	��������
     * param[in] frame			Ҫȡ��ע���ĸ�����µ���غ���
     */
    void UnRegisterJSFunc(const CefString& function_name, int64_t frame_id);

    /**
    * ����ִ�������ķ�ע��һ�������־õ� JS ����
    * param[in] frame			��ǰ�����������
    */
    void UnRegisterJSFuncWithFrame(CefRefPtr<CefFrame> frame);

    /**
     * ��������ִ��ĳ������� JS ����
     * param[in] function_name	��������
     * param[in] json_params	Ҫ���ݵ� json ��ʽ�Ĳ���
     * param[in] frame			ִ���ĸ�����µ� JS ����
     * return ���� true ��ʾ�ɹ�ִ��ĳ�� JS ����
     */
    bool ExecuteJSFunc(const CefString& function_name, const CefString& json_params, CefRefPtr<CefFrame> frame);

  private:
    RenderRegisteredFunction	render_registered_function_;	// ���� JS ���Ѿ�ע��õĳ־ú����б�
  };

}