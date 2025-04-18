// SPDX-FileCopyrightText: 2002-2025 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "PrecompiledHeader.h"

#include "Assertions.h"
#include "ProgressCallback.h"
#include "StringUtil.h"
#include "Console.h"
#include <cmath>
#include <cstdio>
#include <limits>

ProgressCallback::~ProgressCallback() {}

void ProgressCallback::SetFormattedStatusText(const char* Format, ...)
{
	va_list ap;

	va_start(ap, Format);
	const std::string str(StringUtil::StdStringFromFormatV(Format, ap));
	va_end(ap);

	SetStatusText(str.c_str());
}

void ProgressCallback::DisplayFormattedError(const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	const std::string str(StringUtil::StdStringFromFormatV(format, ap));
	va_end(ap);

	DisplayError(str.c_str());
}

void ProgressCallback::DisplayFormattedWarning(const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	const std::string str(StringUtil::StdStringFromFormatV(format, ap));
	va_end(ap);

	DisplayWarning(str.c_str());
}

void ProgressCallback::DisplayFormattedInformation(const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	const std::string str(StringUtil::StdStringFromFormatV(format, ap));
	va_end(ap);

	DisplayInformation(str.c_str());
}

void ProgressCallback::DisplayFormattedDebugMessage(const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	const std::string str(StringUtil::StdStringFromFormatV(format, ap));
	va_end(ap);

	DisplayDebugMessage(str.c_str());
}

void ProgressCallback::DisplayFormattedModalError(const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	const std::string str(StringUtil::StdStringFromFormatV(format, ap));
	va_end(ap);

	ModalError(str.c_str());
}

bool ProgressCallback::DisplayFormattedModalConfirmation(const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	const std::string str(StringUtil::StdStringFromFormatV(format, ap));
	va_end(ap);

	return ModalConfirmation(str.c_str());
}

void ProgressCallback::DisplayFormattedModalInformation(const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	const std::string str(StringUtil::StdStringFromFormatV(format, ap));
	va_end(ap);

	ModalInformation(str.c_str());
}

namespace
{
	class NullProgressCallbacks final : public ProgressCallback
	{
	public:
		void PushState() override {}
		void PopState() override {}

		bool IsCancelled() const override { return false; }
		bool IsCancellable() const override { return false; }

		void SetCancellable(bool cancellable) override {}
		void SetTitle(const char* title) override {}
		void SetStatusText(const char* statusText) override {}
		void SetProgressRange(u32 range) override {}
		void SetProgressValue(u32 value) override {}
		void IncrementProgressValue() override {}
		void SetProgressState(ProgressState state) override {}

		void DisplayError(const char* message) override { Console.Error("%s", message); }
		void DisplayWarning(const char* message) override { Console.Warning("%s", message); }
		void DisplayInformation(const char* message) override { Console.WriteLn("%s", message); }
		void DisplayDebugMessage(const char* message) override { DevCon.WriteLn("%s", message); }

		void ModalError(const char* message) override { Console.Error(message); }
		bool ModalConfirmation(const char* message) override
		{
			Console.WriteLn("%s", message);
			return false;
		}
		void ModalInformation(const char* message) override { Console.WriteLn("%s", message); }
	};
} // namespace

static NullProgressCallbacks s_nullProgressCallbacks;
ProgressCallback* ProgressCallback::NullProgressCallback = &s_nullProgressCallbacks;

std::unique_ptr<ProgressCallback> ProgressCallback::CreateNullProgressCallback()
{
	return std::make_unique<NullProgressCallbacks>();
}

BaseProgressCallback::BaseProgressCallback()
{
}

BaseProgressCallback::~BaseProgressCallback()
{
	State* pNextState = m_saved_state;
	while (pNextState != NULL)
	{
		State* pCurrentState = pNextState;
		pNextState = pCurrentState->next_saved_state;
		delete pCurrentState;
	}
}

void BaseProgressCallback::PushState()
{
	State* pNewState = new State;
	pNewState->cancellable = m_cancellable;
	pNewState->status_text = m_status_text;
	pNewState->progress_range = m_progress_range;
	pNewState->progress_value = m_progress_value;
	pNewState->base_progress_value = m_base_progress_value;
	pNewState->next_saved_state = m_saved_state;
	m_saved_state = pNewState;
}

void BaseProgressCallback::PopState()
{
	pxAssert(m_saved_state);
	State* state = m_saved_state;
	m_saved_state = nullptr;

	// impose the current position into the previous range
	const u32 new_progress_value =
		(m_progress_range != 0) ?
			static_cast<u32>(((float)m_progress_value / (float)m_progress_range) * (float)state->progress_range) :
			state->progress_value;

	m_cancellable = state->cancellable;
	m_status_text = std::move(state->status_text);
	m_progress_range = state->progress_range;
	m_progress_value = new_progress_value;

	m_base_progress_value = state->base_progress_value;
	m_saved_state = state->next_saved_state;
	delete state;
}

bool BaseProgressCallback::IsCancelled() const
{
	return m_cancelled;
}

bool BaseProgressCallback::IsCancellable() const
{
	return m_cancellable;
}

void BaseProgressCallback::SetCancellable(bool cancellable)
{
	m_cancellable = cancellable;
}

void BaseProgressCallback::SetStatusText(const char* text)
{
	m_status_text = text;
}

void BaseProgressCallback::SetProgressRange(u32 range)
{
	if (m_saved_state)
	{
		// impose the previous range on this range
		m_progress_range = m_saved_state->progress_range * range;
		m_base_progress_value = m_progress_value = m_saved_state->progress_value * range;
	}
	else
	{
		m_progress_range = range;
		m_progress_value = 0;
		m_base_progress_value = 0;
	}
}

void BaseProgressCallback::SetProgressValue(u32 value)
{
	SetProgressState(ProgressState::Normal);
	m_progress_value = m_base_progress_value + value;
}

void BaseProgressCallback::IncrementProgressValue()
{
	SetProgressValue((m_progress_value - m_base_progress_value) + 1);
}

void BaseProgressCallback::SetProgressState(ProgressState state)
{
	m_progress_state = state;
}
