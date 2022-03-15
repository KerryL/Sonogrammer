// File:  radioDialog.h
// Date:  3/15/2022
// Auth:  K. Loux
// Desc:  Generic dialog to display several options via radio buttons.

#ifndef RADIO_DIALOG_H_
#define RADIO_DIALOG_H_

// wxWidgets headers
#include <wx/wx.h>

// Standard C++ headers
#include <cassert>

template <typename T>
class RadioDialogItemFactory
{
public:
	virtual size_t GetCount() const = 0;
	virtual wxString GetItemString(const unsigned int& i) const = 0;
	virtual T GetItem(const unsigned int& i) const = 0;
};

template <typename T>
class RadioDialog : public wxDialog
{
public:
	RadioDialog(wxWindow *parent, wxString title, RadioDialogItemFactory<T>& itemFactory)
		: wxDialog(parent, wxID_ANY, title, wxDefaultPosition), itemFactory(itemFactory)
	{
		CreateControls();
		CenterOnParent();
	}
	~RadioDialog() = default;

	T GetSelection() const
	{
		for (size_t i = 0; i < radioButtons.size(); ++i)
		{
			if (radioButtons[i]->GetValue())
				return itemFactory.GetItem(i);
		}

		assert(false);
		return itemFactory.GetItem(0);
	}

private:
	void CreateControls()
	{
		wxSizer* topSizer(new wxBoxSizer(wxHORIZONTAL));
		wxSizer* mainSizer(new wxBoxSizer(wxVERTICAL));
		topSizer->Add(mainSizer, wxSizerFlags().Border(wxALL, 5));

		for (size_t i = 0; i < itemFactory.GetCount(); ++i)
		{
			wxRadioButton* rb = new wxRadioButton(this, wxID_ANY, itemFactory.GetItemString(i));
			mainSizer->Add(rb, wxSizerFlags().Border(wxALL, 5));
			radioButtons.push_back(rb);
		}

		mainSizer->AddSpacer(10);
		wxSizer *buttons(CreateButtonSizer(wxOK | wxCANCEL));
		if (buttons)
			mainSizer->Add(buttons, 0, wxALIGN_CENTER_HORIZONTAL);

		SetSizerAndFit(topSizer);
	}

	RadioDialogItemFactory<T>& itemFactory;

	std::vector<wxRadioButton*> radioButtons;
};

#endif// RADIO_DIALOG_H_
