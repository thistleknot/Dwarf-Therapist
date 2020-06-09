/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef BASETREEWIDGET_H
#define BASETREEWIDGET_H

#include <QTreeWidget>

class QPushButton;

class BaseTreeWidget : public QWidget {
    Q_OBJECT
public:
    BaseTreeWidget(bool requires_refresh, QWidget *parent = nullptr);
    virtual ~BaseTreeWidget();

private:
    bool m_requires_refresh;

protected:
    QTreeWidget *m_tree_view;
    QLineEdit *m_le_search;
    QPushButton *m_btn_toggle_tree;

    QIcon m_arr_in;
    QIcon m_arr_out;
    bool m_collapsed;

    void closeEvent(QCloseEvent *event);

    virtual void search_tree(QString val) = 0;
    virtual void build_tree() = 0;

public slots:
    void refresh();
    void clear();

protected slots:
    void search_changed(QString val);
    void clear_search();
    void toggle_tree();
    virtual void selection_changed() = 0;
    void clear_filter();
};

class SortableTreeItem : public QTreeWidgetItem {
public:
    SortableTreeItem(QTreeWidgetItem* parent = 0)
        : QTreeWidgetItem(parent)
    {}

    static const int TREE_SORT_COL = (int)Qt::UserRole + 50;

private:
    bool operator<(const QTreeWidgetItem &other)const {
        int col = treeWidget()->sortColumn();
        Qt::ItemDataRole idr = (Qt::ItemDataRole)(TREE_SORT_COL + col);
        bool ok = data(col,idr).toInt(&ok);
        if(ok){
            return data(col,idr).toInt() < other.data(col,idr).toInt();
        }else{
            return data(col,idr).toString() < other.data(col,idr).toString();
        }
    }
};

#endif // BASETREEDOCK_H
