/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//kde includes
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kdebug.h>
//qt includes
#include <qlayout.h>
#include <qvbox.h>
//app includes
#include "codegenerationoptionspage.h"
#include "settingsdlg.h"

SettingsDlg::SettingsDlg( QWidget * parent, OptionState state,
			  QDict<GeneratorInfo> ldict, QString activeLanguage)
	: KDialogBase( IconList, i18n("Umbrello Setup"),
        Help | Default | Apply | Ok | Cancel, Ok, parent, 0, true, true ) {
	m_bChangesApplied = false;
	m_OptionState = state;
	setHelp( "umbrello/index.html", QString::null );
	setupGeneralPage();
	setupFontPage();
	setupUIPage();
	setupClassPage();
	setupCodeGenPage(ldict, activeLanguage);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
SettingsDlg::~SettingsDlg() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsDlg::setupUIPage() {
	//setup UI page
	QVBox * page = addVBoxPage( i18n("User Interface"), i18n("User Interface Settings"), DesktopIcon( "window_list") );

	m_UiWidgets.colorGB = new QGroupBox( i18n("Color"), page );
	QGridLayout * colorLayout = new QGridLayout( m_UiWidgets.colorGB, 3, 3 );
	colorLayout -> setSpacing( spacingHint() );
	colorLayout -> setMargin( fontMetrics().height() );

	m_UiWidgets.lineColorL = new QLabel( i18n("Line color:"), m_UiWidgets.colorGB );
	colorLayout -> addWidget( m_UiWidgets.lineColorL, 0, 0 );

	m_UiWidgets.lineColorB = new KColorButton( m_OptionState.uiState.lineColor, m_UiWidgets.colorGB );
	colorLayout -> addWidget( m_UiWidgets.lineColorB, 0, 1 );

	m_UiWidgets.lineDefaultB = new QPushButton( i18n("D&efault Color"), m_UiWidgets.colorGB );
	colorLayout -> addWidget( m_UiWidgets.lineDefaultB, 0, 2 );

	m_UiWidgets.fillColorL = new QLabel( i18n("Fill color:"), m_UiWidgets.colorGB );
	colorLayout -> addWidget( m_UiWidgets.fillColorL, 1, 0 );

	m_UiWidgets.fillColorB = new KColorButton( m_OptionState.uiState.fillColor, m_UiWidgets.colorGB );
	colorLayout -> addWidget( m_UiWidgets.fillColorB, 1, 1 );

	m_UiWidgets.fillDefaultB = new QPushButton( i18n("De&fault Color"), m_UiWidgets.colorGB );
	colorLayout -> addWidget( m_UiWidgets.fillDefaultB, 1, 2 );

	m_UiWidgets.useFillColorCB = new QCheckBox( i18n("&Use fill color"), m_UiWidgets.colorGB );
	colorLayout -> setRowStretch( 2, 2 );
	colorLayout -> addWidget( m_UiWidgets.useFillColorCB, 2, 0 );
	m_UiWidgets.useFillColorCB -> setChecked( m_OptionState.uiState.useFillColor );

	//connect button signals up
	connect( m_UiWidgets.lineDefaultB, SIGNAL(clicked()), this, SLOT(slotLineBClicked()) );
	connect( m_UiWidgets.fillDefaultB, SIGNAL(clicked()), this, SLOT(slotFillBClicked()) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsDlg::setupGeneralPage() {
	//setup General page

	QVBox * page = addVBoxPage( i18n("General"), i18n("General Settings"), DesktopIcon( "misc")  );

	//setup autosave settings

	m_GeneralWidgets.autosaveGB = new QGroupBox( i18n("Autosave"), page );

	QGridLayout * autosaveLayout = new QGridLayout( m_GeneralWidgets.autosaveGB, 2, 2 );
	autosaveLayout -> setSpacing( spacingHint() );
	autosaveLayout -> setMargin( fontMetrics().height() );

	m_GeneralWidgets.autosaveCB = new QCheckBox( i18n("E&nable autosave"), m_GeneralWidgets.autosaveGB );
	m_GeneralWidgets.autosaveCB -> setChecked( m_OptionState.generalState.autosave );
	autosaveLayout -> addWidget( m_GeneralWidgets.autosaveCB, 0, 0 );

	m_GeneralWidgets.autosaveL = new QLabel( i18n("Select auto-save time interval (mins):"), m_GeneralWidgets.autosaveGB );


	autosaveLayout -> addWidget( m_GeneralWidgets.autosaveL, 1, 0 );

	m_GeneralWidgets.timeKB = new KComboBox( m_GeneralWidgets.autosaveGB );
	m_GeneralWidgets.timeKB -> setEnabled( m_OptionState.generalState.autosave );
	autosaveLayout -> addWidget( m_GeneralWidgets.timeKB, 1, 1 );

	QString time[4] = { i18n("5"), i18n("10"), i18n("15"), i18n("30") };
	for( int i=0; i < 4; i++ )
		m_GeneralWidgets.timeKB -> insertItem( time[i] );
	m_GeneralWidgets.timeKB -> setDuplicatesEnabled( false );
	m_GeneralWidgets.timeKB -> setCurrentItem( m_OptionState.generalState.time );

	//setup startup settings
	m_GeneralWidgets.startupGB = new QGroupBox( i18n("Startup"), page );

	QGridLayout * startupLayout = new QGridLayout( m_GeneralWidgets.startupGB, 3, 2 );
	startupLayout -> setSpacing( spacingHint() );
	startupLayout -> setMargin( fontMetrics().height() );

	m_GeneralWidgets.logoCB = new QCheckBox( i18n("Sta&rtup logo"), m_GeneralWidgets.startupGB );
	m_GeneralWidgets.logoCB -> setChecked( m_OptionState.generalState.logo );
	startupLayout -> addWidget( m_GeneralWidgets.logoCB, 0, 0 );

	m_GeneralWidgets.tipCB = new QCheckBox( i18n("&Tip of the day"), m_GeneralWidgets.startupGB );
	m_GeneralWidgets.tipCB -> setChecked( m_OptionState.generalState.tip );
	startupLayout -> addWidget( m_GeneralWidgets.tipCB, 0, 1 );

	m_GeneralWidgets.loadlastCB = new QCheckBox( i18n("&Load last project"), m_GeneralWidgets.startupGB );
	m_GeneralWidgets.loadlastCB -> setChecked( m_OptionState.generalState.loadlast );
	startupLayout -> addWidget( m_GeneralWidgets.loadlastCB, 1, 0 );

	m_GeneralWidgets.startL = new QLabel( i18n("Start new project with:"), m_GeneralWidgets.startupGB );
	startupLayout -> addWidget( m_GeneralWidgets.startL, 2, 0 );

	m_GeneralWidgets.diagramKB = new KComboBox( m_GeneralWidgets.startupGB );
	startupLayout -> addWidget( m_GeneralWidgets.diagramKB, 2, 1 );

	QString diagrams [] = { i18n("No Diagram"), i18n("Class Diagram"),
	                        i18n("Use Case Diagram"), i18n("Sequence Diagram"),
				i18n("Collaboration Diagram"), i18n("State Diagram"),
				i18n("Activity Diagram"), i18n("Component Diagram"),
				i18n("Deployment Diagram") };
	for( int i=0; i<9; i++)
		m_GeneralWidgets.diagramKB -> insertItem( diagrams[i] );

	m_GeneralWidgets.diagramKB -> setCurrentItem( (int) m_OptionState.generalState.diagram );
	connect( m_GeneralWidgets.autosaveCB, SIGNAL(clicked()), this, SLOT(slotAutosaveCBClicked()) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsDlg::setupClassPage() {
	//setup class settings page

	QVBox * page = addVBoxPage( i18n("Class"), i18n("Class Settings"), DesktopIcon( "edit")  );
	m_ClassWidgets.visibilityGB = new QGroupBox( i18n("Visibility"), page );

	QGridLayout * visibilityLayout = new QGridLayout( m_ClassWidgets.visibilityGB );
	visibilityLayout -> setSpacing( spacingHint() );
	visibilityLayout -> setMargin(  fontMetrics().height()  );

	m_ClassWidgets.showScopeCB = new QCheckBox(i18n("Show &visibility"), m_ClassWidgets.visibilityGB);
	m_ClassWidgets.showScopeCB -> setChecked(  m_OptionState.classState.showScope );
	visibilityLayout -> addWidget( m_ClassWidgets.showScopeCB, 0, 0 );

	m_ClassWidgets.showAttsCB = new QCheckBox( i18n("Show attributes"), m_ClassWidgets.visibilityGB );
	m_ClassWidgets.showAttsCB -> setChecked(  m_OptionState.classState.showAtts );
	visibilityLayout -> addWidget( m_ClassWidgets.showAttsCB, 0, 1 );

	m_ClassWidgets.showOpsCB = new QCheckBox( i18n("Show operations"), m_ClassWidgets.visibilityGB );
	m_ClassWidgets.showOpsCB -> setChecked(  m_OptionState.classState.showOps );
	visibilityLayout -> addWidget( m_ClassWidgets.showOpsCB, 1, 0 );

	m_ClassWidgets.showStereotypeCB = new QCheckBox( i18n("Show stereot&ype"), m_ClassWidgets.visibilityGB );
	m_ClassWidgets.showStereotypeCB -> setChecked(  m_OptionState.classState.showStereoType );
	visibilityLayout -> addWidget( m_ClassWidgets.showStereotypeCB, 1, 1 );

	m_ClassWidgets.showAttSigCB = new QCheckBox(i18n("Show attribute signature"), m_ClassWidgets.visibilityGB);
	m_ClassWidgets.showAttSigCB -> setChecked(   m_OptionState.classState.showAttSig );
	visibilityLayout -> addWidget( m_ClassWidgets.showAttSigCB, 2, 0 );


	m_ClassWidgets.showPackageCB = new QCheckBox(i18n("Show package"), m_ClassWidgets.visibilityGB);
	m_ClassWidgets.showPackageCB -> setChecked(  m_OptionState.classState.showPackage );
	visibilityLayout -> addWidget( m_ClassWidgets.showPackageCB, 2, 1 );

	m_ClassWidgets.showOpSigCB = new QCheckBox( i18n("Show operation signature"), m_ClassWidgets.visibilityGB );
	m_ClassWidgets.showOpSigCB -> setChecked(  m_OptionState.classState.showOpSig );
	visibilityLayout -> addWidget( m_ClassWidgets.showOpSigCB, 3, 0 );
	visibilityLayout -> setRowStretch( 3, 1 );
}

void SettingsDlg::setupCodeGenPage(QDict<GeneratorInfo> ldict, QString activeLanguage) {
	//setup code generation settings page
	QVBox * page = addVBoxPage( i18n("Code Generation"), i18n("Code Generation Settings"), DesktopIcon( "source") );
	m_pCodeGenPage = new CodeGenerationOptionsPage(m_OptionState.codegenState, ldict,
						       activeLanguage, page);
}

void SettingsDlg::setupFontPage() {
	QVBox * page = addVBoxPage( i18n("Font"), i18n("Font Settings"), DesktopIcon( "fonts")  );
	m_FontWidgets.chooser = new KFontChooser( page, "font", false, QStringList(), false);

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsDlg::slotApply() {
	applyPage( (Page) activePageIndex() );
	emit applyClicked();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsDlg::slotOk() {
	applyPage( page_general );
	applyPage( page_font );
	applyPage( page_UI );
	applyPage( page_class );
	applyPage( page_codegen );
	accept();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsDlg::slotDefault() {
	/*
	   Defaults hard coded.  Make sure that this is alright.
	   If defaults are set anywhere else, like in setting up config file, make sure the same.
	*/
	switch( activePageIndex() ) {
		case page_general:
			m_GeneralWidgets.autosaveCB -> setChecked( false );
			m_GeneralWidgets.timeKB -> setCurrentItem( 0 );
			m_GeneralWidgets.logoCB -> setChecked( true );
			m_GeneralWidgets.tipCB -> setChecked( true );
			m_GeneralWidgets.loadlastCB -> setChecked( true );
			m_GeneralWidgets.diagramKB -> setCurrentItem( 0 );
			break;

		case page_font:
			m_FontWidgets.chooser -> setFont( parentWidget() -> font() );
			break;

		case page_UI:
			m_UiWidgets.useFillColorCB -> setChecked( true );
			m_UiWidgets.fillColorB -> setColor( QColor( 255, 255, 192 ) );
			m_UiWidgets.lineColorB -> setColor( red );
			break;

		case page_class:
			m_ClassWidgets.showScopeCB -> setChecked( false );
			m_ClassWidgets.showAttsCB -> setChecked( true );
			m_ClassWidgets.showOpsCB -> setChecked( true );
			m_ClassWidgets.showStereotypeCB -> setChecked( false );
			m_ClassWidgets.showAttSigCB -> setChecked( false );
			m_ClassWidgets.showOpSigCB -> setChecked( false );
			m_ClassWidgets.showPackageCB -> setChecked( false );
			break;

		case page_codegen:
			m_pCodeGenPage -> setDefaults();
			break;
	};
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsDlg::applyPage( Page page ) {
	m_bChangesApplied = true;
	switch( page ) {
		case page_general:
			m_OptionState.generalState.autosave = m_GeneralWidgets.autosaveCB -> isChecked();
			m_OptionState.generalState.time = m_GeneralWidgets.timeKB -> currentItem();
			m_OptionState.generalState.logo = m_GeneralWidgets.logoCB -> isChecked();
			m_OptionState.generalState.tip = m_GeneralWidgets.tipCB -> isChecked();
			m_OptionState.generalState.loadlast = m_GeneralWidgets.loadlastCB -> isChecked();
			m_OptionState.generalState.diagram  = ( Diagram ) m_GeneralWidgets.diagramKB -> currentItem();
			break;

		case page_font:
			m_OptionState.uiState.font = m_FontWidgets.chooser -> font();
			break;

		case page_UI:
			m_OptionState.uiState.useFillColor = m_UiWidgets.useFillColorCB -> isChecked();
			m_OptionState.uiState.fillColor = m_UiWidgets.fillColorB -> color();
			m_OptionState.uiState.lineColor = m_UiWidgets.lineColorB -> color();
			break;

		case page_class:
			m_OptionState.classState.showScope = m_ClassWidgets.showScopeCB -> isChecked();
			m_OptionState.classState.showAtts = m_ClassWidgets.showAttsCB -> isChecked();
			m_OptionState.classState.showOps = m_ClassWidgets.showOpsCB -> isChecked();
			m_OptionState.classState.showStereoType = m_ClassWidgets.showStereotypeCB -> isChecked();
			m_OptionState.classState.showAttSig = m_ClassWidgets.showAttSigCB -> isChecked();
			m_OptionState.classState.showOpSig = m_ClassWidgets.showOpSigCB -> isChecked();
			m_OptionState.classState.showPackage = m_ClassWidgets.showPackageCB -> isChecked();
			break;

		case page_codegen:
			m_pCodeGenPage->state(m_OptionState.codegenState);
			break;
	}
}
void SettingsDlg::slotLineBClicked() {
	m_UiWidgets.lineColorB -> setColor( red );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsDlg::slotFillBClicked() {
	m_UiWidgets.fillColorB -> setColor( QColor(255, 255, 192) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SettingsDlg::slotAutosaveCBClicked() {
	m_GeneralWidgets.timeKB -> setEnabled( m_GeneralWidgets.autosaveCB -> isChecked() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString SettingsDlg::getCodeGenerationLanguage() {
	return m_pCodeGenPage->getCodeGenerationLanguage();
}










#include "settingsdlg.moc"
