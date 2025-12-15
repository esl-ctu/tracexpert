#include "tprotocolwidget.h"

TProtocolWidget::TProtocolWidget(TProtocolContainer * protocolContainer, QWidget * parent)
    : TProjectUnitWidget(protocolContainer, parent)
{
    m_fileTypeFilter = "TraceXpert protocol file (*.txpp)";
}

