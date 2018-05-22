#include "IGameBaseFrameworkModule.h"

class FGameBaseFrameworkModule : public IGameBaseFrameworkModule
{
public:

    FGameBaseFrameworkModule();

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual bool IsGameModule() const override
    {
        return true;
    }
private:

};

IMPLEMENT_MODULE( FGameBaseFrameworkModule, GameBaseFramework )

FGameBaseFrameworkModule::FGameBaseFrameworkModule()
{

}

void FGameBaseFrameworkModule::StartupModule()
{
}

void FGameBaseFrameworkModule::ShutdownModule()
{
}