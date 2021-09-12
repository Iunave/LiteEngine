#include "Damageable.hpp"

FColor IDamageable::ConvertArmourToColor() const
{
    const float64_4 Percentage{Simd::SetAll<float64_4>(CurrentArmour / MaxArmour)};
    FColor ColorModifier{0.4, 0.4, 0.4, 0.0};

    ColorModifier = Simd::FusedMultiplyAdd(ColorModifier.Data(), Percentage, 1.0_float64_4);

    return DefaultColor * ColorModifier;
}
