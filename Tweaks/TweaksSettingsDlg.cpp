#include "TweaksSettingsDlg.h"
#include "tweaks_settings.h"
#include "workspace.h"
#include <wx/choicdlg.h>
#include "windowattrmanager.h"
#include "macros.h"
#include <wx/msgdlg.h>
#include <wx/richmsgdlg.h>

TweaksSettingsDlg::TweaksSettingsDlg(wxWindow* parent)
    : TweaksSettingsDlgBase(parent)
{
    m_settings.Load();
    m_checkBoxEnableTweaks->SetValue( m_settings.IsEnableTweaks() );

    DoPopulateList();
    WindowAttrManager::Load(this, "TweaksSettingsDlg", NULL);
}

TweaksSettingsDlg::~TweaksSettingsDlg()
{
    WindowAttrManager::Save(this, "TweaksSettingsDlg", NULL);
}

void TweaksSettingsDlg::OnWorkspaceOpenUI(wxUpdateUIEvent& event)
{
    event.Enable( WorkspaceST::Get()->IsOpen() );
}

void TweaksSettingsDlg::DoPopulateList()
{
    // Get list of projects
    wxArrayString projects;
    WorkspaceST::Get()->GetProjectList( projects );

    // ----------------------------------------------------------------------
    // Add tab colours properties
    // ----------------------------------------------------------------------
    if ( m_settings.GetGlobalBgColour().IsOk() ) {
        wxVariant value;
        value << m_settings.GetGlobalBgColour();
        m_pgMgrTabColours->SetPropertyValue( m_pgPropGlobalTabBG, value );
    }

    if ( m_settings.GetGlobalFgColour().IsOk() ) {
        wxVariant value;
        value << m_settings.GetGlobalFgColour();
        m_pgMgrTabColours->SetPropertyValue( m_pgPropGlobalTabFG, value );
    }

    for(size_t i=0; i<projects.GetCount(); ++i) {
        const ProjectTweaks& pt = m_settings.GetProjectTweaks(projects.Item(i));
        // Image table
        m_pgMgr->Append( new wxFileProperty(projects.Item(i), wxPG_LABEL, pt.IsOk() ? pt.GetBitmapFilename() : ""));

        // Colours table
        wxPGProperty *parentProject = m_pgMgrTabColours->AppendIn(m_pgPropProjectsColours, new wxPropertyCategory(projects.Item(i)));
        wxString labelBG, labelFG;
        labelBG << projects.Item(i) << " background colour";
        labelFG << projects.Item(i) << " text colour";
        if ( pt.IsOk() ) {
            m_pgMgrTabColours->AppendIn(parentProject, new wxSystemColourProperty(labelBG, wxPG_LABEL, pt.GetTabBgColour() ));
            m_pgMgrTabColours->AppendIn(parentProject, new wxSystemColourProperty(labelFG, wxPG_LABEL, pt.GetTabFgColour() ));
        } else {
            m_pgMgrTabColours->AppendIn(parentProject, new wxSystemColourProperty(labelBG));
            m_pgMgrTabColours->AppendIn(parentProject, new wxSystemColourProperty(labelFG));
        }
    }
}

void TweaksSettingsDlg::OnEnableTweaks(wxCommandEvent& event)
{
    m_settings.SetEnableTweaks( event.IsChecked() );
}

void TweaksSettingsDlg::OnEnableTweaksUI(wxUpdateUIEvent& event)
{
    event.Enable( m_checkBoxEnableTweaks->IsChecked() && WorkspaceST::Get()->IsOpen() );
}

void TweaksSettingsDlg::OnEnableTweaksCheckboxUI(wxUpdateUIEvent& event)
{
    event.Enable( WorkspaceST::Get()->IsOpen() );
}

void TweaksSettingsDlg::OnColourChanged(wxPropertyGridEvent& event)
{
    wxPGProperty* prop = event.GetProperty();
    CHECK_PTR_RET(prop);

    if ( prop == m_pgPropGlobalTabBG ) {
        // Global tab bg colour was modified
        wxColourPropertyValue cpv;
        cpv << prop->GetValue();
        m_settings.SetGlobalBgColour( cpv.m_colour );

    } else if ( prop == m_pgPropGlobalTabFG ) {
        // Global tab colour was modified
        wxColourPropertyValue cpv;
        cpv << prop->GetValue();
        m_settings.SetGlobalFgColour( cpv.m_colour );

    } else if ( prop->GetParent() ) {
        // project specific colour was changed
        wxColourPropertyValue cpv;
        cpv << prop->GetValue();

        if ( prop->GetLabel().Contains("text colour") ) {
            m_settings.GetProjectTweaks(prop->GetParent()->GetLabel()).SetTabFgColour( cpv.m_colour );

        } else if ( prop->GetLabel().Contains("background colour") ) {
            m_settings.GetProjectTweaks(prop->GetParent()->GetLabel()).SetTabBgColour( cpv.m_colour );

        }
    }
}
void TweaksSettingsDlg::OnImageSelected(wxPropertyGridEvent& event)
{
    wxPGProperty* prop = event.GetProperty();
    CHECK_PTR_RET(prop);
    
    wxString projectName = prop->GetLabel();
    m_settings.GetProjectTweaks(projectName).SetBitmapFilename( prop->GetValueAsString() );
    
    if ( !m_settings.HasFlag( TweaksSettings::kDontPromptForProjectReload ) ) {
        wxRichMessageDialog dlg(this, _("Icon changes require a workspace reload"), "CodeLite", wxOK|wxOK_DEFAULT|wxCANCEL|wxICON_INFORMATION);
        dlg.ShowCheckBox(_("Remember my answer"));
        if ( dlg.ShowModal() == wxID_OK ) {
            if ( dlg.IsCheckBoxChecked() ) {
                m_settings.EnableFlag( TweaksSettings::kDontPromptForProjectReload, true );
            }
        }
    }
}