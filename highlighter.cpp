/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <davyjones@github>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::black);
    rule.pattern = QRegExp("\\b[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);
    keywordFormat.setForeground(QColor(100,10,100));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bselect\\b" << "\\bupdate\\b" << "\\bdelete\\b"
                    << "\\btruncate\\b" << "\\bunion\\b" << "\\ball\\b"
                    << "\\bintersect\\b" << "\\bexcept\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern, Qt::CaseInsensitive);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
    keywordFormat2.setForeground(Qt::darkBlue);
    //keywordFormat2.setFontItalic(true);
    QStringList keywordPatterns2;
    keywordPatterns2 << "\\bfrom\\b" << "\\bin\\b" << "\\bwith\\b" << "\\bwhere\\b"
                     << "\\bjoin\\b" << "\\bon\\b" << "\\band\\b" << "\\bgroup by\\b"
                     << "\\bleft\\b" << "\\right\\b" << "\\bfull\\b" << "\\bcross\\b"
                     << "\\binner\\b" << "\\bouter\\b" << "\\bnatural\\b"
                     << "\\border by\\b" << "\\blimit\\b" << "\\bfetch\\b"
                     << "\\bhaving\\b" << "\\bwindow\\b" << "\\boffset\\b";
    foreach (const QString &pattern, keywordPatterns2) {
        rule.pattern = QRegExp(pattern, Qt::CaseInsensitive);
        rule.format = keywordFormat2;
        highlightingRules.append(rule);
    }
    singleQuoteFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\'([^\']*)\'");
    rule.format = singleQuoteFormat;
    highlightingRules.append(rule);
    doubleQuoteFormat.setForeground(Qt::darkCyan);
    rule.pattern = QRegExp("\"([^\"]*)\"");
    rule.format = doubleQuoteFormat;
    highlightingRules.append(rule);
    functionFormat.setFontWeight(QFont::Bold);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
    singleLineCommentFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegExp("--[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
}
