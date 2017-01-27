/*
  Q Light Controller
  vcframeproperties.cpp

  Copyright (c) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QCheckBox>
#include <QComboBox>
#include <algorithm>

#include "inputselectionwidget.h"
#include "vcframepageshortcut.h"
#include "vcframeproperties.h"
#include "vcframe.h"
#include "doc.h"


VCFrameProperties::VCFrameProperties(QWidget* parent, VCFrame* frame, Doc *doc)
    : QDialog(parent)
    , m_frame(frame)
    , m_doc(doc)
{
    Q_ASSERT(frame != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_frameName->setText(frame->caption());
    m_allowChildrenCheck->setChecked(frame->allowChildren());
    m_allowResizeCheck->setChecked(frame->allowResize());
    m_showHeaderCheck->setChecked(frame->isHeaderVisible());
    m_showEnableButtonCheck->setChecked(frame->isEnableButtonVisible());
    m_enablePaging->setChecked(frame->multipageMode());
    m_pagesLoopCheck->setChecked(frame->pagesLoop());
    if (frame->multipageMode() == true)
        m_showHeaderCheck->setEnabled(false);
    m_totalPagesSpin->setValue(frame->totalPagesNumber());
    if (frame->totalPagesNumber() != 1)
        m_cloneFirstPageCheck->setEnabled(false);

    connect(m_enablePaging, SIGNAL(toggled(bool)),
            this, SLOT(slotMultipageChecked(bool)));

    /************************************************************************
     * Enable frame
     ************************************************************************/

    m_inputEnableWidget = new InputSelectionWidget(m_doc, this);
    m_inputEnableWidget->setTitle(tr("External Input - Enable"));
    m_inputEnableWidget->setCustomFeedbackVisibility(true);
    m_inputEnableWidget->setKeySequence(m_frame->enableKeySequence());
    m_inputEnableWidget->setInputSource(m_frame->inputSource(VCFrame::enableInputSourceId));
    m_inputEnableWidget->setWidgetPage(m_frame->page());
    m_inputEnableWidget->show();
    m_extEnableLayout->addWidget(m_inputEnableWidget);

    /************************************************************************
     * Previous page
     ************************************************************************/

    m_inputPrevPageWidget = new InputSelectionWidget(m_doc, this);
    m_inputPrevPageWidget->setTitle(tr("External Input - Previous Page"));
    m_inputPrevPageWidget->setCustomFeedbackVisibility(true);
    m_inputPrevPageWidget->setKeySequence(m_frame->previousPageKeySequence());
    m_inputPrevPageWidget->setInputSource(m_frame->inputSource(VCFrame::previousPageInputSourceId));
    m_inputPrevPageWidget->setWidgetPage(m_frame->page());
    m_inputPrevPageWidget->show();
    m_extInputPages->addWidget(m_inputPrevPageWidget);

    /************************************************************************
     * Next page
     ************************************************************************/

    m_inputNextPageWidget = new InputSelectionWidget(m_doc, this);
    m_inputNextPageWidget->setTitle(tr("External Input - Next Page"));
    m_inputNextPageWidget->setCustomFeedbackVisibility(true);
    m_inputNextPageWidget->setKeySequence(m_frame->nextPageKeySequence());
    m_inputNextPageWidget->setInputSource(m_frame->inputSource(VCFrame::nextPageInputSourceId));
    m_inputNextPageWidget->setWidgetPage(m_frame->page());
    m_inputNextPageWidget->show();
    m_extInputPages->addWidget(m_inputNextPageWidget);

    /************************************************************************
     * Page shortcuts
     ************************************************************************/
    foreach(VCFramePageShortcut const* shortcut, m_frame->shortcuts())
        m_shortcuts.append(new VCFramePageShortcut(*shortcut));

    m_pageSelect = new QComboBox(this);
    slotTotalPagesNumberChanged(m_totalPagesSpin->value());
    m_pageSelect->setStyleSheet("QLabel { background-color: #000000; font-size: 15px; font-weight: bold;"
                               "color: red; border-radius: 3px; padding: 3px; margin-left: 2px; }");
    m_extInputPages->addWidget(m_pageSelect);

    m_shortcutInputWidget = new InputSelectionWidget(m_doc, this);
    m_shortcutInputWidget->setCustomFeedbackVisibility(true);
    m_shortcutInputWidget->setWidgetPage(m_frame->page());
    m_shortcutInputWidget->show();
    m_extInputPages->addWidget(m_shortcutInputWidget);
    slotPageSelectChanged(0);

    connect(m_totalPagesSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotTotalPagesNumberChanged(int)));

    connect(m_shortcutInputWidget, SIGNAL(inputValueChanged(quint32,quint32)),
            this, SLOT(slotInputValueChanged(quint32,quint32)));
    connect(m_shortcutInputWidget, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(slotKeySequenceChanged(QKeySequence)));

    connect (m_pageSelect, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotPageSelectChanged(int)));
}

VCFrameProperties::~VCFrameProperties()
{
    foreach (VCFramePageShortcut* shortcut, m_shortcuts)
    {
        delete shortcut;
    }
}

bool VCFrameProperties::allowChildren() const
{
    return m_allowChildrenCheck->isChecked();
}

bool VCFrameProperties::allowResize() const
{
    return m_allowResizeCheck->isChecked();
}

bool VCFrameProperties::showHeader() const
{
    return m_showHeaderCheck->isChecked();
}

QString VCFrameProperties::frameName() const
{
    return m_frameName->text();
}

bool VCFrameProperties::multipageEnabled() const
{
    return m_enablePaging->isChecked();
}

bool VCFrameProperties::cloneWidgets() const
{
    return m_cloneFirstPageCheck->isChecked();
}

bool VCFrameProperties::pagesLoop() const
{
    return m_pagesLoopCheck->isChecked();
}

void VCFrameProperties::slotMultipageChecked(bool enable)
{
    if (enable == true)
    {
        m_showHeaderCheck->setChecked(true);
        m_showHeaderCheck->setEnabled(false);
    }
}

void VCFrameProperties::slotPageSelectChanged(int index)
{
    if (index >= 0 && m_shortcuts[index] != NULL)
    {
        m_shortcutInputWidget->setTitle(tr("Page: %1").arg(index + 1));
        m_shortcutInputWidget->setInputSource(m_shortcuts[index]->m_inputSource);
        m_shortcutInputWidget->setKeySequence(m_shortcuts[index]->m_keySequence.toString(QKeySequence::NativeText));
    }
}

void VCFrameProperties::slotTotalPagesNumberChanged(int number)
{
    m_pageSelect->clear();
    // Add pages if number was increased, remove them if decreased
    for (int i = 0; i < std::max(m_shortcuts.length(), number); i++)
    {
        if (i < number)
        {
            // Update page selection for input widget
            m_pageSelect->addItem(tr("Page: %1").arg(i + 1));
            if (m_shortcuts.length() <= i)
            {
                VCFramePageShortcut* vcfps = new VCFramePageShortcut(i);
                m_shortcuts.append(vcfps);
            }
        }
        else
            m_shortcuts.removeAt(i);
    }
}

void VCFrameProperties::slotInputValueChanged(quint32 universe, quint32 channel)
{
    Q_UNUSED(universe);
    Q_UNUSED(channel);

    VCFramePageShortcut *shortcut = m_shortcuts[m_pageSelect->currentIndex()];

    if (shortcut != NULL)
        shortcut->m_inputSource = m_shortcutInputWidget->inputSource();
}

void VCFrameProperties::slotKeySequenceChanged(QKeySequence key)
{
    VCFramePageShortcut *shortcut = m_shortcuts[m_pageSelect->currentIndex()];

    if (shortcut != NULL)
        shortcut->m_keySequence = key;
}


void VCFrameProperties::accept()
{
    bool hasHeader = m_frame->isHeaderVisible();

    m_frame->setCaption(m_frameName->text());
    m_frame->setAllowChildren(m_allowChildrenCheck->isChecked());
    m_frame->setAllowResize(m_allowResizeCheck->isChecked());

    /* If the frame is coming from a headerless state,
     * all the children widgets must be moved down */
    if (m_showHeaderCheck->isChecked() && hasHeader == false)
    {
        QListIterator <VCWidget*> it(m_frame->findChildren<VCWidget*>());

        // resize the frame too if it contains children
        if (it.hasNext())
            m_frame->resize(QSize(m_frame->width(), m_frame->height() + 40));

        while (it.hasNext() == true)
        {
            VCWidget* child = it.next();

            // move only first level children
            if (child->parentWidget() == m_frame)
                child->move(QPoint(child->x(), child->y() + 40));
        }
    }
    m_frame->setHeaderVisible(m_showHeaderCheck->isChecked());
    m_frame->setEnableButtonVisible(m_showEnableButtonCheck->isChecked());
    m_frame->setMultipageMode(m_enablePaging->isChecked());
    m_frame->setTotalPagesNumber(m_totalPagesSpin->value());
    m_frame->setPagesLoop(m_pagesLoopCheck->isChecked());

    /* Key sequences */
    m_frame->setEnableKeySequence(m_inputEnableWidget->keySequence());
    m_frame->setNextPageKeySequence(m_inputNextPageWidget->keySequence());
    m_frame->setPreviousPageKeySequence(m_inputPrevPageWidget->keySequence());

    /* Input sources */
    m_frame->setInputSource(m_inputEnableWidget->inputSource(), VCFrame::enableInputSourceId);
    m_frame->setInputSource(m_inputNextPageWidget->inputSource(), VCFrame::nextPageInputSourceId);
    m_frame->setInputSource(m_inputPrevPageWidget->inputSource(), VCFrame::previousPageInputSourceId);

    /* Shortcuts */
    m_frame->resetShortcuts();
    for (int i = 0; i < m_shortcuts.count(); i++)
        m_frame->addShortcut(*m_shortcuts.at(i));

    QDialog::accept();
}

