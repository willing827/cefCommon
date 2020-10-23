#pragma once

#include <functional>
#include "include/cef_app.h"

typedef std::function<void(const std::string& params)> CppFunction;

typedef std::map<std::pair<CefString /*function_name*/, int /*browser_id*/>, CppFunction/* function*/> BrowserRegisteredFunction;

class CefJSBridge
{
public:
  CefJSBridge();
  ~CefJSBridge();
public:

  /**
   * ִ���Ѿ�ע��õ� JS ����
   * param[in] js_function_name Ҫ���õ� JS ��������
   * param[in] params			���� JS �������ݵ� json ��ʽ����
   * param[in] browser	  �����ĸ�������µ� JS ����
   * return ���� ture ��ʶ����ִ�� JS ��������ɹ�
   */
  bool CallJSFunction(const CefString& js_function_name, const CefString& params, CefRefPtr<CefFrame> frame);

  /**
   * ע��һ���־õ� C++ �����ṩ JS �˵���
   * param[in] function_name	Ҫ�ṩ JS ���õĺ�������
   * param[in] func		������
   */
  void RegisterCppFunc(const CefString& function_name, CppFunction func, int browser_id);

  /**
   * ��ע��һ���־õ� C++ ����
   * param[in] function_name	Ҫ��ע��ĺ�������
   */
  void UnRegisterCppFunc(const CefString& function_name, int browser_id);

  /**
   * ִ��һ���Ѿ�ע��õ� C++ ���������ܵ� JS ��ִ������ʱ�����ã�
   * param[in] function_name	Ҫִ�еĺ�������
   * param[in] params			Я���Ĳ���
   * param[in] browser		browser ʵ�����
   * return ���� true ��ʾִ�гɹ������� false ��ʾִ��ʧ�ܣ����������ܲ�����
   */
  bool ExecuteCppFunc(const CefString& function_name, const CefString& params, int browser_id);

private:
  BrowserRegisteredFunction	browser_registered_function_;	// ���� C++ ���Ѿ�ע��õĳ־ú����б�
};