#pragma once

#include <animaBaseIsotropicCompartment.h>
#include <AnimaMCMExport.h>

namespace anima
{

class ANIMAMCM_EXPORT IsotropicRestrictedWaterCompartment : public BaseIsotropicCompartment
{
public:
    // Useful typedefs
    typedef IsotropicRestrictedWaterCompartment Self;
    typedef BaseIsotropicCompartment Superclass;
    typedef Superclass::Pointer BasePointer;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    // New macro
    itkNewMacro(Self)

    /** Run-time type information (and related methods) */
    itkTypeMacro(IsotropicRestrictedWaterCompartment, BaseIsotropicCompartment)

    DiffusionModelCompartmentType GetCompartmentType() ITK_OVERRIDE {return IsotropicRestrictedWater;}

    virtual ListType &GetParameterLowerBounds() ITK_OVERRIDE;
    virtual ListType &GetParameterUpperBounds() ITK_OVERRIDE;

protected:
    IsotropicRestrictedWaterCompartment() : Superclass()
    {
        this->SetAxialDiffusivity(1.0e-3);
    }

    virtual ~IsotropicRestrictedWaterCompartment() {}
};

} // end namespace anima
