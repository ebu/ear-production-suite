#ifndef BLOCKBUILDERS_H
#define BLOCKBUILDERS_H
#include <chrono>
#include <adm/adm.hpp>

namespace admplug::testing {

   class ObjectTypeBlock {
   public:
       virtual ~ObjectTypeBlock() = default;
       virtual operator adm::AudioBlockFormatObjects() const = 0;
   };


   class CartesianCoordBlock : public ObjectTypeBlock {
      public:
       CartesianCoordBlock();
       CartesianCoordBlock withX(double x);
       CartesianCoordBlock withY(double y);
       CartesianCoordBlock withZ(double z);
       CartesianCoordBlock withDuration(double seconds);
       CartesianCoordBlock withRtime(double seconds);
       CartesianCoordBlock withJumpPosition(bool flag);
       CartesianCoordBlock withJumpPosition(bool flag, std::chrono::nanoseconds);
       operator adm::AudioBlockFormatObjects () const;

      private:
       template<typename T>
       CartesianCoordBlock setCartesianElement(double value) {
           CartesianCoordBlock builder{*this};
           auto pos = builder.block.get<adm::CartesianPosition>();
           pos.set(T{static_cast<float>(value)});
           builder.block.set(pos);
           return builder;
       }
       adm::AudioBlockFormatObjects block;
   };
   CartesianCoordBlock initialCartesianBlock();

   class SphericalCoordBlock : public ObjectTypeBlock {
   public:
       SphericalCoordBlock();
       SphericalCoordBlock withAzimuth(double azimuth);
       SphericalCoordBlock withElevation(double elevation);
       SphericalCoordBlock withDistance(double distance);
       SphericalCoordBlock withDuration(double seconds);
       SphericalCoordBlock withRtime(double seconds);
       SphericalCoordBlock withGain(double linearGain);
       SphericalCoordBlock withJumpPosition(bool flag);
       SphericalCoordBlock withJumpPosition(bool flag, std::chrono::nanoseconds length);
       operator adm::AudioBlockFormatObjects () const;

   private:
       template<typename T>
       SphericalCoordBlock setSphericalElement(double value){
           SphericalCoordBlock builder{*this};
           auto pos = builder.block.get<adm::SphericalPosition>();
           pos.set(T{static_cast<float>(value)});
           builder.block.set(pos);
           return builder;
       }

       adm::AudioBlockFormatObjects block;
   };

   SphericalCoordBlock initialSphericalBlock();

   class ObjectTypeBlockRange {
   public:
       ObjectTypeBlockRange();
       ObjectTypeBlockRange with(ObjectTypeBlock const& block);
       ObjectTypeBlockRange followedBy(ObjectTypeBlock const& block);
       operator adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects> ();
       adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects> asConstRange();

   private:
       template<typename T, typename U>
       auto getValOrDefault(U parent) -> typename T::value_type {
           typename T::value_type val{0};
           if(parent.template has<T>()) {
               val = parent.template get<T>().get();
           }
           return val;
       }
       template<typename T, typename U>
       adm::Time getValOrDefaultTime(U parent) {
         if(parent.template has<T>()) {
           return parent.template get<T>().get();
         }
         return T(std::chrono::nanoseconds::zero()).get();
       }



     std::vector<adm::AudioBlockFormatObjects> blocks;

   };
}

#endif // BLOCKBUILDERS_H
