#pragma once
#include "ImGuiManager.h"


class ImGuiDialogBox
{
protected:

	// ImGuiManager Event reference and our handle to the append so we can remove it in destructor
	eventpp::CallbackList<void()>& pImGuiRenderEvent;
	eventpp::CallbackList<void()>::Handle mCallbackHandle = {};

	// What we run when ImGuiManager ImGuiRenderEvent is invoked
	virtual void onImGuiRenderEvent() {}

public:
	explicit ImGuiDialogBox(eventpp::CallbackList<void()>& renderEvent) : pImGuiRenderEvent(renderEvent) {}
	 virtual ~ImGuiDialogBox()
	{
		 PLOG_DEBUG << "~ImGuiDialogBox()";
		pImGuiRenderEvent.remove(mCallbackHandle);
	}
};

class ErrorMessageBox : public ImGuiDialogBox
{
private:
	std::string mErrorMessage;
	void onImGuiRenderEvent() final;
public:
	explicit ErrorMessageBox(eventpp::CallbackList<void()>& renderEvent, std::string errorMessage)
		: ImGuiDialogBox(renderEvent), mErrorMessage(errorMessage) 
	{
		mCallbackHandle = pImGuiRenderEvent.append([this]() {ImGuiDialogBox::onImGuiRenderEvent(); });
	}


};