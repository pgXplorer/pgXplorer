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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>

struct ParenthesisInfo
{
    QChar character;
    int position;
};

class TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData(){}

    QVector<ParenthesisInfo *> parentheses()
    {
        return m_parentheses;
    }

    void insert(ParenthesisInfo *info)
    {
        int i = 0;
        while (i < m_parentheses.size() &&
            info->position > m_parentheses.at(i)->position)
            ++i;

        m_parentheses.insert(i, info);
    }

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

class Highlighter : public QSyntaxHighlighter
{
     Q_OBJECT

 public:
     Highlighter(QTextDocument *parent = 0);

 protected:
     void highlightBlock(const QString &text);

 private:
     struct HighlightingRule
     {
         QRegExp pattern;
         QTextCharFormat format;
     };
     QVector<HighlightingRule> highlightingRules;

     QRegExp comment_start_exp;
     QRegExp comment_end_exp;

     QTextCharFormat keywordFormat;
     QTextCharFormat keywordFormat2;
     QTextCharFormat classFormat;
     QTextCharFormat singleLineCommentFormat;
     QTextCharFormat multiLineCommentFormat;
     QTextCharFormat singleQuoteFormat;
     QTextCharFormat doubleQuoteFormat;
     QTextCharFormat functionFormat;
};

#endif // HIGHLIGHTER_H
