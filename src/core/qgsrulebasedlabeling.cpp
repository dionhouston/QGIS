#include "qgsrulebasedlabeling.h"



QgsRuleBasedLabelProvider::QgsRuleBasedLabelProvider( const QgsRuleBasedLabeling& rules, QgsVectorLayer* layer, bool withFeatureLoop )
    : QgsVectorLayerLabelProvider( layer, withFeatureLoop )
    , mRules( rules )
{
  mRules.rootRule()->createSubProviders( layer, mSubProviders );
}

QgsRuleBasedLabelProvider::~QgsRuleBasedLabelProvider()
{
  // sub-providers owned by labeling engine
}


bool QgsRuleBasedLabelProvider::prepare( const QgsRenderContext& context, QStringList& attributeNames )
{
  Q_FOREACH ( QgsVectorLayerLabelProvider* provider, mSubProviders.values() )
    provider->setEngine( mEngine );

  // populate sub-providers
  mRules.rootRule()->prepare( context, attributeNames, mSubProviders );
  return true;
}

void QgsRuleBasedLabelProvider::registerFeature( QgsFeature& feature, const QgsRenderContext& context )
{
  // will register the feature to relevant sub-providers
  mRules.rootRule()->registerFeature( feature, context, mSubProviders );
}

QList<QgsAbstractLabelProvider*> QgsRuleBasedLabelProvider::subProviders()
{
  QList<QgsAbstractLabelProvider*> lst;
  Q_FOREACH ( QgsVectorLayerLabelProvider* subprovider, mSubProviders.values() )
    lst << subprovider;
  return lst;
}


////////////////////

QgsRuleBasedLabeling::Rule::Rule( QgsPalLayerSettings* settings, int scaleMinDenom, int scaleMaxDenom, const QString& filterExp, const QString& label, const QString& description, bool elseRule )
    : mParent( 0 ), mSettings( settings )
    , mScaleMinDenom( scaleMinDenom ), mScaleMaxDenom( scaleMaxDenom )
    , mFilterExp( filterExp ), mLabel( label ), mDescription( description )
    , mElseRule( elseRule )
    //, mIsActive( true )
    , mFilter( 0 )
{
  initFilter();
}

QgsRuleBasedLabeling::Rule::~Rule()
{
  delete mSettings;
  delete mFilter;
  qDeleteAll( mChildren );
  // do NOT delete parent
}

void QgsRuleBasedLabeling::Rule::initFilter()
{
  if ( mElseRule || mFilterExp.compare( "ELSE", Qt::CaseInsensitive ) == 0 )
  {
    mElseRule = true;
    mFilter = 0;
  }
  else if ( !mFilterExp.isEmpty() )
  {
    delete mFilter;
    mFilter = new QgsExpression( mFilterExp );
  }
  else
  {
    mFilter = 0;
  }
}


void QgsRuleBasedLabeling::Rule::appendChild( QgsRuleBasedLabeling::Rule* rule )
{
  mChildren.append( rule );
  rule->mParent = this;
  // TODO updateElseRules();
}

QgsRuleBasedLabeling::Rule*QgsRuleBasedLabeling::Rule::clone() const
{
  QgsPalLayerSettings* s = mSettings ? new QgsPalLayerSettings( *mSettings ) : 0;
  Rule* newrule = new Rule( s, mScaleMinDenom, mScaleMaxDenom, mFilterExp, mLabel, mDescription );
  // TODO newrule->setCheckState( mIsActive );
  // clone children
  Q_FOREACH ( Rule* rule, mChildren )
    newrule->appendChild( rule->clone() );
  return newrule;
}

void QgsRuleBasedLabeling::Rule::createSubProviders( QgsVectorLayer* layer, QgsRuleBasedLabeling::RuleToProviderMap& subProviders )
{
  if ( mSettings )
  {
    // add provider!
    QgsVectorLayerLabelProvider* p = new QgsVectorLayerLabelProvider( layer, false, mSettings );
    subProviders[this] = p;
  }

  // call recursively
  Q_FOREACH ( Rule* rule, mChildren )
  {
    rule->createSubProviders( layer, subProviders );
  }
}

void QgsRuleBasedLabeling::Rule::prepare( const QgsRenderContext& context, QStringList& attributeNames, QgsRuleBasedLabeling::RuleToProviderMap& subProviders )
{
  if ( mSettings )
  {
    QgsVectorLayerLabelProvider* p = subProviders[this];
    if ( !p->prepare( context, attributeNames ) )
    {
      subProviders.remove( this );
      delete p;
    }
  }

  if ( mFilter )
    mFilter->prepare( &context.expressionContext() );

  // call recursively
  Q_FOREACH ( Rule* rule, mChildren )
  {
    rule->prepare( context, attributeNames, subProviders );
  }
}

void QgsRuleBasedLabeling::Rule::registerFeature( QgsFeature& feature, const QgsRenderContext& context, QgsRuleBasedLabeling::RuleToProviderMap& subProviders )
{
  bool isOK = isFilterOK( feature, const_cast<QgsRenderContext&>( context ) );  // TODO: remove const_cast

  if ( !isOK )
    return;

  // do we have active subprovider for the rule?
  if ( subProviders.contains( this ) )
    subProviders[this]->registerFeature( feature, context );

  // call recursively
  Q_FOREACH ( Rule* rule, mChildren )
  {
    rule->registerFeature( feature, context, subProviders );
  }
}

bool QgsRuleBasedLabeling::Rule::isFilterOK( QgsFeature& f, QgsRenderContext& context ) const
{
  if ( ! mFilter || mElseRule )
    return true;

  context.expressionContext().setFeature( f );
  QVariant res = mFilter->evaluate( &context.expressionContext() );
  return res.toInt() != 0;
}

////////////////////

QgsRuleBasedLabeling::QgsRuleBasedLabeling( QgsRuleBasedLabeling::Rule* root )
    : mRootRule( root )
{

}

QgsRuleBasedLabeling::QgsRuleBasedLabeling( const QgsRuleBasedLabeling& other )
{
  mRootRule = other.mRootRule->clone();
}

QgsRuleBasedLabeling::~QgsRuleBasedLabeling()
{
  delete mRootRule;
}