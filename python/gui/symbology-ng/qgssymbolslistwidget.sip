class QgsSymbolsListWidget : QWidget
{
%TypeHeaderCode
#include <qgssymbolslistwidget.h>
%End
  public:
    QgsSymbolsListWidget( QgsSymbolV2* symbol, QgsStyleV2* style, QMenu* menu, QWidget* parent /TransferThis/ = 0, const QgsVectorLayer * layer = 0 );

    /** Returns the expression context used for the widget, if set. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the list widget.
     * @note added in QGIS 2.12
     * @see setExpressionContext()
     */
    QgsExpressionContext* expressionContext() const;

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.12
     */
    virtual void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas
     * @note added in QGIS 2.12
     */
    const QgsMapCanvas* mapCanvas() const;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* layer() const;

  public slots:
    void setSymbolFromStyle( const QModelIndex & index );
    void setSymbolColor( const QColor& color );
    void setMarkerAngle( double angle );
    void setMarkerSize( double size );
    void setLineWidth( double width );
    void addSymbolToStyle();
    void symbolAddedToStyle( QString name, QgsSymbolV2* symbol );
    void on_mSymbolUnitWidget_changed();
    void on_mTransparencySlider_valueChanged( int value );

    void on_groupsCombo_currentIndexChanged( int index );
    void on_groupsCombo_editTextChanged( const QString &text );

    void openStyleManager();

  signals:
    void changed();

  protected:
    void populateSymbolView();
    void populateSymbols( QStringList symbols );
    void updateSymbolColor();
    void updateSymbolInfo();
};
