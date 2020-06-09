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
#include "skilllegendwidget.h"
#include "statetableview.h"
#include "gamedatareader.h"
#include "dwarfmodel.h"
#include "columntypes.h"
#include "rotatedheader.h"
#include "dwarftherapist.h"
#include "optionsmenu.h"
#include "defines.h"
#include "viewcolumn.h"
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>

SkillLegendWidget::SkillLegendWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    QHBoxLayout *l_type = new QHBoxLayout();
    QLabel *lbl_type = new QLabel(this);
    lbl_type->setText(tr("Drawing Method:"));
    l_type->addWidget(lbl_type);
    QComboBox *cmb_type = new QComboBox(this);
    for(int i = 0; i < UberDelegate::SDM_TOTAL_METHODS; ++i) {
        UberDelegate::SKILL_DRAWING_METHOD m = static_cast<UberDelegate::SKILL_DRAWING_METHOD>(i);
        cmb_type->addItem(QString("Use %1").arg(UberDelegate::name_for_method(m)),m);
    }
    l_type->addWidget(cmb_type);
    layout->addLayout(l_type);

    StateTableView *stv = new StateTableView(this);
    stv->setContextMenuPolicy(Qt::ActionsContextMenu);

    layout->addWidget(stv);

    QStandardItemModel *m = new QStandardItemModel(this);
    stv->setModel(m);
    for(int i = 20; i >= 0; --i) {
        QList<QStandardItem*> sub_items;
        QStandardItem *name = new QStandardItem;
        name->setText(GameDataReader::ptr()->get_skill_level_name(i));
        QStandardItem *item = new QStandardItem;
        float rating = (float)i + 0.002;
        item->setData(rating, DwarfModel::DR_RATING);
        item->setData(floor(rating), DwarfModel::DR_DISPLAY_RATING);
        item->setData(CT_SKILL, DwarfModel::DR_COL_TYPE);
        item->setData(QColor(Qt::white), DwarfModel::DR_DEFAULT_BG_COLOR);
        item->setData(ViewColumn::STATE_TOGGLE, DwarfModel::DR_STATE);
        sub_items << name << item;
        m->appendRow(sub_items);
    }
    stv->setHeaderHidden(true);

    int pad = DT->user_settings()->value("options/grid/cell_padding", 0).toInt();
    int cell_size = DT->user_settings()->value("options/grid/cell_size", DEFAULT_CELL_SIZE).toInt();
    cell_size += (2 + 2*pad);

    for(int i = 1; i < 16; ++i) {
        stv->get_header()->resizeSection(i, cell_size);
    }

    m_current_method = static_cast<UberDelegate::SKILL_DRAWING_METHOD>(
                DT->user_settings()->value("options/grid/skill_drawing_method",
                                           UberDelegate::SDM_NUMERIC).toInt());
    cmb_type->setCurrentIndex(m_current_method);

    connect(cmb_type, SIGNAL(currentIndexChanged(int)), this, SLOT(set_SDM(int)));
    connect(this, SIGNAL(change_skill_drawing_method(const UberDelegate::SKILL_DRAWING_METHOD&)),
        DT->get_options_menu(), SLOT(set_skill_drawing_method(const UberDelegate::SKILL_DRAWING_METHOD&)));
}

void SkillLegendWidget::set_SDM(int idx){
    QComboBox *c = qobject_cast<QComboBox*>(QObject::sender());
    UberDelegate::SKILL_DRAWING_METHOD sdm = static_cast<UberDelegate::SKILL_DRAWING_METHOD>(c->itemData(idx, Qt::UserRole).toInt());
    if(sdm != m_current_method){
        emit change_skill_drawing_method(sdm);
        m_current_method = sdm;
    }
}
