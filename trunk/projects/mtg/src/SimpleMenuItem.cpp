#include "../include/config.h"
#include "../include/SimpleMenuItem.h"
#include "../include/Translate.h"
#include "../include/WResourceManager.h"

SimpleMenuItem::SimpleMenuItem(SimpleMenu* _parent, int id, int fontId, string text, int x, int y, bool hasFocus, bool autoTranslate): JGuiObject(id), parent(_parent), fontId(fontId), mX(x), mY(y)
{
  if (autoTranslate) mText = _(text);
  else mText = text;
  mHasFocus = hasFocus;

  mScale = 1.0f;
  mTargetScale = 1.0f;

  if (hasFocus)
    Entering();
}


void SimpleMenuItem::RenderWithOffset(float yOffset)
{
  JLBFont * mFont = resources.GetJLBFont(fontId);
  //mFont->SetColor(ARGB(255,255,255,255));
  mFont->DrawString(mText.c_str(), mX, mY + yOffset, JGETEXT_CENTER);
}

void SimpleMenuItem::Render()
{
  RenderWithOffset(0);
}

void SimpleMenuItem::Update(float dt)
{
    if (mScale < mTargetScale)
    {
      mScale += 8.0f*dt;
      if (mScale > mTargetScale)
	mScale = mTargetScale;
    }
  else if (mScale > mTargetScale)
    {
      mScale -= 8.0f*dt;
      if (mScale < mTargetScale)
	mScale = mTargetScale;
	}
}


void SimpleMenuItem::Entering()
{
  mHasFocus = true;
  parent->selectionTargetY = mY;
}


bool SimpleMenuItem::Leaving(JButton key)
{
  mHasFocus = false;
  return true;
}


bool SimpleMenuItem::ButtonPressed()
{
  return true;
}

void SimpleMenuItem::Relocate(int x, int y)
{
  mX = x;
  mY = y;
}

int SimpleMenuItem::GetWidth()
{
  JLBFont * mFont = resources.GetJLBFont(fontId);
  mFont->SetScale(1.0);
  return mFont->GetStringWidth(mText.c_str());
}

bool SimpleMenuItem::hasFocus()
{
  return mHasFocus;
}

ostream& SimpleMenuItem::toString(ostream& out) const
{
  return out << "SimpleMenuItem ::: mHasFocus : " << mHasFocus
	     << " ; parent : " << parent
	     << " ; mText : " << mText
	     << " ; mScale : " << mScale
	     << " ; mTargetScale : " << mTargetScale
	     << " ; mX,mY : " << mX << "," << mY;
}
