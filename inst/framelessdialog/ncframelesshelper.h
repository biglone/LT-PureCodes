#ifndef NC_FRAMELESS_HELPER_H
#define NC_FRAMELESS_HELPER_H

#include <QObject>

class NcFramelessHelperImpl;

class NcFramelessHelper : public QObject
{

public:
  explicit NcFramelessHelper( QObject* parent = 0 );
  ~NcFramelessHelper();

  void activateOn( QWidget* topLevelWidget );
  void removeFrom( QWidget* topLevelWidget );

  void setWidgetMovable( bool movable );
  bool isWidgetMovable();

  void setWidgetResizable( bool resizable );
  bool isWidgetResizable();

  void useRubberBandOnMove( bool use );
  bool isUsingRubberBandOnMove();

  void useRubberBandOnResize( bool use );
  bool isUsingRubberBandOnResisze();

  //Make sure to leave the same content margins
  //around the widget as the newBorderWidth
  //this can be done by
  //yourWidget->layout()->setMargin( newBorderWidth );
  //otherwise your widget will not expose the
  //area where this class works
  void setBorderWidth( int newBorderWidth );
  int borderWidth();

protected:
  virtual bool eventFilter( QObject* obj, QEvent* event );

private:
  NcFramelessHelperImpl* d;
};

#endif // NC_FRAMELESS_HELPER_H
