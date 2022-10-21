#pragma once

#include <QWidget>

class QTextEdit;

class TextLogWidget : public QWidget
{
	Q_OBJECT

public:
	TextLogWidget(QWidget* parent, size_t line_limit = 10000);

	void append(const QString& message);

private:
	size_t line_count = 0;
	size_t line_limit = 1;

	QTextEdit* text_edit = nullptr;
};
