// SpectrumPlotWndProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "SpectrumPlotWndProperty.h"
#include "SeriesObj.h"
#include "PlotWndPropSpectrum.h"
#include "AppMessage.h"

// SpectrumPlotWndProperty

IMPLEMENT_DYNAMIC(SpectrumPlotWndProperty, BaseProperty)

SpectrumPlotWndProperty::SpectrumPlotWndProperty()
{

}

SpectrumPlotWndProperty::~SpectrumPlotWndProperty()
{
}


BEGIN_MESSAGE_MAP(SpectrumPlotWndProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// SpectrumPlotWndProperty message handlers

int SpectrumPlotWndProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	PlotWndPropSpectrum * objProp = (PlotWndPropSpectrum *)this->target;

	CMFCPropertyGridProperty * pProp;
	
	pProp = new CMFCPropertyGridProperty(_T("Type"), _T("Spectrum Window"));
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Name"), objProp->GetName());
	this->AddProperty(pProp);
	
	pProp = new CMFCPropertyGridProperty(_T("X Label"), objProp->GetXLabel());
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Y Label"), objProp->GetYLabel());
	this->AddProperty(pProp);

	VARIANT_BOOL boolAutoScale;
	if (objProp->autoScale)
		boolAutoScale = VARIANT_TRUE;
	else
		boolAutoScale = VARIANT_FALSE;

	COleVariant varAutoScale((short)boolAutoScale, VT_BOOL);
	pProp = new CMFCPropertyGridProperty(_T("Auto Scale"), varAutoScale, _T("Enable/disable auto scale function for plot window"));
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Max Value"), (_variant_t) (int)objProp->GetMaxValue(), _T("Specifies the max value."));
	this->AddProperty(pProp);
	if (objProp->autoScale)
		pProp->Enable(FALSE);

	pProp = new CMFCPropertyGridProperty( _T("Min Value"), (_variant_t) (int)objProp->GetMinValue(), _T("Specifies the min value"));
	this->AddProperty(pProp);
	if (objProp->autoScale)
		pProp->Enable(FALSE);

	VARIANT_BOOL boolShowGrid;
	if (objProp->showGrid)
		boolShowGrid = VARIANT_TRUE;
	else
		boolShowGrid = VARIANT_FALSE;

	COleVariant varShowGrid((short)boolShowGrid, VT_BOOL);
	pProp = new CMFCPropertyGridProperty(_T("Show Grid"), varShowGrid, _T("Enable/disable grid on plot window"));
	this->AddProperty(pProp);

	CString strMode;
	if (objProp->isLog)
		strMode.SetString(L"Logarithm");
	else
		strMode.SetString(L"General");

	pProp = new CMFCPropertyGridProperty(_T("Coordinate"), strMode.GetBuffer(), _T("Specifies the type of the coordinate system"));
	pProp->AddOption(_T("General"));
	pProp->AddOption(_T("Logarithm"));
	pProp->AllowEdit(FALSE);

	this->AddProperty(pProp);

	return 0;
}

void SpectrumPlotWndProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class
	PlotWndPropSpectrum * objProp = (PlotWndPropSpectrum *)this->target;

	if (wcscmp(pProp->GetName(), L"Name") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->SetName(value.bstrVal);
		objProp->nameIsSet = true;
	}
	else if (wcscmp(pProp->GetName(), L"Y Label") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->YLabel(value.bstrVal);
	}
	else if (wcscmp(pProp->GetName(), L"X Label") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->XLabel(value.bstrVal);
	}
	else if (wcscmp(pProp->GetName(), L"Show Grid") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->showGrid = (value.boolVal != 0);
	}
	else if (wcscmp(pProp->GetName(), L"Auto Scale") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->autoScale = (value.boolVal != 0);
		if (objProp->autoScale)
		{
			objProp->autoScaleReset = true;
		}

		CMFCPropertyGridProperty * propertyMax = FindProperty(L"Max Value");
		CMFCPropertyGridProperty * propertyMin = FindProperty(L"Min Value");
		BOOL enableEdit = objProp->autoScale ? FALSE : TRUE;

		propertyMax->Enable(enableEdit);
		propertyMin->Enable(enableEdit);
	}
	else if (wcscmp(pProp->GetName(), L"Max Value") == 0)
	{
		_variant_t value = pProp->GetValue();
		if (value.intVal < objProp->GetMinValue() + 1)
		{
			value.intVal = (int)objProp->GetMinValue() + 1;
			pProp->SetValue(value);
		}
		objProp->SetMaxValue(value.intVal);
	}
	else if (wcscmp(pProp->GetName(), L"Min Value") == 0)
	{
		_variant_t value = pProp->GetValue();
		if (value.intVal > objProp->GetMaxValue() - 1)
		{
			value.intVal = (int)objProp->GetMaxValue() - 1;
			pProp->SetValue(value);
		}
		objProp->SetMinValue(value.intVal);
		if (!objProp->autoScale)
			objProp->SetDispMinValue(objProp->GetMinValue());
	}
	else if (wcscmp(pProp->GetName(), L"Coordinate") == 0)
	{
		_variant_t value = pProp->GetValue();
		BSTR bstr = value.bstrVal;
		if ( wcscmp(bstr, L"General") == 0 )
		{
			objProp->isLog = false;
		}
		else
		{
			objProp->isLog = true;
		}

		if (objProp->autoScale)
		{
			objProp->autoScaleReset = true;
		}
	}

	objProp->ForceUpdateGraph();

	return BaseProperty::OnPropertyChanged(pProp);
}

void SpectrumPlotWndProperty::UpdatePropertyValue(const wchar_t * propertyName)
{

	CMFCPropertyGridProperty * property = NULL;
	for (int i = 0; i < this->GetPropertyCount(); i++)
	{
		 CMFCPropertyGridProperty * aProperty = this->GetProperty(i);
		 if (wcscmp(aProperty->GetName(), propertyName) == 0)
		 {
			 property = aProperty;
			 break;
		 }
	}

	if (property == NULL)
		return;

	_variant_t value = property->GetValue();
	
	PlotWndPropSpectrum * objProp = (PlotWndPropSpectrum *)this->target;
	if (wcscmp(propertyName, L"Max Value") == 0)
	{
		_variant_t value = property->GetValue();
		value.intVal = (int)objProp->GetMaxValue();
		property->SetValue(value);
	}
	else if (wcscmp(propertyName, L"Min Value") == 0)
	{
		_variant_t value = property->GetValue();
		value.intVal = (int)objProp->GetMinValue();
		property->SetValue(value);
	}
	else if (wcscmp(propertyName, L"Auto Scale") == 0)
	{
		_variant_t value = property->GetValue();
		value.boolVal = objProp->autoScale;
		property->SetValue(value);

		CMFCPropertyGridProperty * propertyMax = FindProperty(L"Max Value");
		CMFCPropertyGridProperty * propertyMin = FindProperty(L"Min Value");
		BOOL enableEdit = objProp->autoScale ? FALSE : TRUE;

		propertyMax->Enable(enableEdit);
		propertyMin->Enable(enableEdit);
	}
}
