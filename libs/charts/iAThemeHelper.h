// Copyright 2016-2023, the open_iA contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "iAcharts_export.h"

class QIcon;
class QString;

//! Helpers for determining whether currently a bright or dark theme is applied, and for retrieving icons fitting the current theme
namespace iAThemeHelper
{
	//! Retrieve an icon for a given name from the embedded resources; implementation currently in iAModuleDispatcher.cpp
	iAcharts_API QIcon icon(QString const& name);
	//! set bright mode (enabled=true) or dark mode (enabled=false)
	//! @param enabled whether we are switching to bright mode (true) or dark mode (false)
	iAcharts_API void setBrightMode(bool enabled);
	//! whether we are currently in bright mode
	iAcharts_API bool brightMode();
}