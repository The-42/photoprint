/*
 * signature.h - class to handle coordinate calculations for n-up rectangular layouts
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "pageextent.h"
#include "layoutrectangle.h"

#define DEFAULTGUTTER 15


class LayoutRectangle;

class Signature : public virtual PageExtent
{
	public:
	Signature(int rows=1,int columns=1);
	Signature(PageExtent &extent,int rows=1,int columns=1);
	~Signature()
	{
	}
	void SetPageExtent(PageExtent &pe);
	void SetPaperSize(int width,int height);
	void SetMargins(int left,int right,int top,int bottom);
	void SetGutters(int hgutter,int vgutter);
	void SetHGutter(int gutter);
	void SetVGutter(int gutter);
	void SetColumns(int columns);
	void SetRows(int rows);
	void SetCellWidth(int width);
	void SetCellHeight(int height);
	int GetCellWidth();
	int GetCellHeight();
	int GetColumns();
	int GetRows();
	int GetHGutter();
	int GetVGutter();
	bool GetAbsolute();
	LayoutRectangle *GetLayoutRectangle(int row,int column);
	void EqualiseMargins();
	void ReCalc();
	void ReCalcByCellSize();
	int ColumnAt(int xpos);
	int RowAt(int ypos);
	protected:
	int hgutter,vgutter;
	int rows,columns;
	float celwidth,celheight;
	int rightpadding,bottompadding;
	bool absolutemode;				// Used to track whether we're recalcing in terms of rows/columns or cell size.
};

#endif
