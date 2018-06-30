#include <iostream>

#include "xSlicerInterpreter.h"
#include "xeus/xguid.hpp"

#include <qMRMLSliceWidget.h>
#include <qMRMLSliceView.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDView.h>

#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerPythonManager.h>

#include <QBuffer>

void xSlicerInterpreter::configure_impl()
{
    auto handle_comm_opened = [](xeus::xcomm&& comm, const xeus::xmessage&) {
        std::cout << "Comm opened for target: " << comm.target().name() << std::endl;
    };
    comm_manager().register_comm_target("echo_target", handle_comm_opened);
    //using function_type = std::function<void(xeus::xcomm&&, const xeus::xmessage&)>;
}

xjson xSlicerInterpreter::execute_request_impl(int execution_counter,
                                             const std::string& code,
                                             bool silent,
                                             bool store_history,
                                             const xjson_node* /* user_expressions */,
                                             bool allow_stdin)
{
    std::cout << "Received execute_request" << std::endl;
    std::cout << "execution_counter: " << execution_counter << std::endl;
    std::cout << "code: " << code << std::endl;
    std::cout << "silent: " << silent << std::endl;
    std::cout << "store_history: " << store_history << std::endl;
    std::cout << "allow_stdin: " << allow_stdin << std::endl;
    std::cout << std::endl;

    qSlicerPythonManager* pythonManager = qSlicerApplication::application()->pythonManager();

    xjson pub_data;
    QString qscode = QString::fromUtf8(code.c_str());
    QString displayCommand = "display()";
    if (qscode.endsWith(displayCommand))
    {
      QVariant executeResult = pythonManager->executeString(qscode.left(qscode.length()-displayCommand.length()));

      // Make sure display updates are completed
      qSlicerApplication::application()->processEvents();
      qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
      for (int viewIndex = 0; viewIndex < layoutManager->threeDViewCount(); viewIndex++)
      {
        layoutManager->threeDWidget(viewIndex)->threeDView()->forceRender();
      }
      foreach(QString sliceViewName, layoutManager->sliceViewNames())
      {
        layoutManager->sliceWidget(sliceViewName)->sliceView()->forceRender();
      }

      // base64 encoding
      QPixmap screenshot = QPixmap::grabWidget(layoutManager->viewport());
      QByteArray bArray;
      QBuffer buffer(&bArray);
      buffer.open(QIODevice::WriteOnly);
      screenshot.save(&buffer, "PNG");
      QString base64 = QString::fromLatin1(bArray.toBase64().data());

      pub_data["image/png"] = base64.toStdString();
    }
    else
    {
      QVariant executeResult = pythonManager->executeString(QString::fromStdString(code));
      pub_data["text/plain"] = executeResult.toString().toStdString();
    }

    publish_execution_result(execution_counter, std::move(pub_data), xjson());

    xjson result;
    result["status"] = "ok";
    return result;
}

xjson xSlicerInterpreter::complete_request_impl(const std::string& code,
                                              int cursor_pos)
{
    std::cout << "Received complete_request" << std::endl;
    std::cout << "code: " << code << std::endl;
    std::cout << "cursor_pos: " << cursor_pos << std::endl;
    std::cout << std::endl;
    xjson result;
    result["status"] = "ok";
    result["matches"] = {"a.echo1"};
    result["cursor_start"] = 2;
    result["cursor_end"] = 6;
    return result;
}

xjson xSlicerInterpreter::inspect_request_impl(const std::string& code,
                                             int cursor_pos,
                                             int detail_level)
{
    std::cout << "Received inspect_request" << std::endl;
    std::cout << "code: " << code << std::endl;
    std::cout << "cursor_pos: " << cursor_pos << std::endl;
    std::cout << "detail_level: " << detail_level << std::endl;
    std::cout << std::endl;
    xjson result;
    result["status"] = "ok";
    result["found"] = false;
    return result;
}

xjson xSlicerInterpreter::history_request_impl(const xhistory_arguments& args)
{
    std::cout << "Received history_request" << std::endl;
    std::cout << "output: " << args.m_output << std::endl;
    std::cout << "raw: " << args.m_raw << std::endl;
    std::cout << "hist_access_type: " << args.m_hist_access_type << std::endl;
    std::cout << "session: " << args.m_session << std::endl;
    std::cout << "start: " << args.m_start << std::endl;
    std::cout << "stop: " << args.m_stop << std::endl;
    std::cout << "n: " << args.m_n << std::endl;
    std::cout << "pattern: " << args.m_pattern << std::endl;
    std::cout << "unique: " << args.m_unique << std::endl;
    std::cout << std::endl;
    xjson result;
    result["history"] = {{args.m_session, 0, ""}};
    return result;
}

xjson xSlicerInterpreter::is_complete_request_impl(const std::string& code)
{
    std::cout << "Received is_complete_request" << std::endl;
    std::cout << "code: " << code << std::endl;
    std::cout << std::endl;
    xjson result;
    result["status"] = "complete";
    return result;
}

xjson xSlicerInterpreter::kernel_info_request_impl()
{
    xjson result;
    result["implementation"] = "cpp_echo";
    result["implementation_version"] = "1.0.0";
    result["language_info"]["name"] = "cpp";
    result["language_info"]["version"] = "14.0.0";
    result["language_info"]["mimetype"] = "text/x-c++src";
    result["language_info"]["file_extension"] = ".cpp";
    return result;
}

void xSlicerInterpreter::input_reply_impl(const std::string& value)
{
    std::cout << "Received input_reply" << std::endl;
    std::cout << "value: " << value << std::endl;
}
