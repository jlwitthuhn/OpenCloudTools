#include "widget_text_log.h"

#include <QMargins>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

TextLogWidget::TextLogWidget(QWidget* const parent, const size_t line_limit) : QWidget{ parent }, line_limit { line_limit }
{
	text_edit = new QTextEdit{ this };

	QVBoxLayout* const layout = new QVBoxLayout{ this };
	layout->setContentsMargins(QMargins{ 0, 0, 0, 0 });

	layout->addWidget(text_edit);
}

void TextLogWidget::append(const QString& message)
{
	if (line_count == line_limit)
	{
		QTextCursor cursor = text_edit->textCursor();
		cursor.movePosition(QTextCursor::Start);
		cursor.select(QTextCursor::LineUnderCursor);
		// Delete the line (except newline), then newline
		cursor.removeSelectedText();
		cursor.deleteChar();
		line_count--;
	}
	text_edit->append(message);
	line_count++;
}
