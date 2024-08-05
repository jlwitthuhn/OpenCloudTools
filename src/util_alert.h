#pragma once

#include <string>

class QWidget;

void alert_error_blocking(const std::string& title, const std::string& message, QWidget* parent = nullptr);
