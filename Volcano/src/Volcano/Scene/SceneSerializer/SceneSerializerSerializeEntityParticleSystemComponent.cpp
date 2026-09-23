#include "volpch.h"
#include "SceneSerializer.h"

#include "Volcano/Scene/Entity.h"
#include "Volcano/Utils/YAMLUtils.h"

namespace Volcano
{
	namespace Utils
	{

		void SerializeParticleSystemComponent(YAML::Emitter& out, Entity& entity)
		{
			out << YAML::Key << "ParticleSystemComponent";
			out << YAML::BeginMap; // ParticleSystemComponent
			{
				auto& particleSystemComponent = entity.GetComponent<ParticleSystemComponent>();

				out << YAML::Key << "enabled" << YAML::Value << particleSystemComponent.enabled;

				auto& particleSystem = particleSystemComponent.particleSystem;

				out << YAML::Key << "duration"               << YAML::Value << particleSystem->duration;                           
				out << YAML::Key << "looping"                << YAML::Value << particleSystem->looping;                            
				out << YAML::Key << "prewarm"                << YAML::Value << particleSystem->prewarm;                            
				out << YAML::Key << "startDelay1"            << YAML::Value << particleSystem->startDelay1;
				out << YAML::Key << "startDelay2"            << YAML::Value << particleSystem->startDelay2;
				out << YAML::Key << "startDelayType"         << YAML::Value << particleSystem->startDelayType;                     
				out << YAML::Key << "startLifetime1"         << YAML::Value << particleSystem->startLifetime1;
				out << YAML::Key << "startLifetime2"         << YAML::Value << particleSystem->startLifetime2;
				out << YAML::Key << "startLifetimeType"      << YAML::Value << particleSystem->startLifetimeType;                  
				out << YAML::Key << "startSpeed1"            << YAML::Value << particleSystem->startSpeed1;
				out << YAML::Key << "startSpeed2"            << YAML::Value << particleSystem->startSpeed2;
				out << YAML::Key << "startSpeedType"         << YAML::Value << particleSystem->startSpeedType;                     
				out << YAML::Key << "useThreeDStartSize"     << YAML::Value << particleSystem->useThreeDStartSize;                 
				out << YAML::Key << "threeDStartSize1"       << YAML::Value << particleSystem->threeDStartSize1;
				out << YAML::Key << "threeDStartSize2"       << YAML::Value << particleSystem->threeDStartSize2;
				out << YAML::Key << "startSize1"             << YAML::Value << particleSystem->startSize1;
				out << YAML::Key << "startSize2"             << YAML::Value << particleSystem->startSize2;
				out << YAML::Key << "startSizeType"          << YAML::Value << particleSystem->startSizeType;                      
				out << YAML::Key << "useThreeDStartRotation" << YAML::Value << particleSystem->useThreeDStartRotation;             
				out << YAML::Key << "threeDStartRotation1"   << YAML::Value << particleSystem->threeDStartRotation1;
				out << YAML::Key << "threeDStartRotation2"   << YAML::Value << particleSystem->threeDStartRotation2;
				out << YAML::Key << "startRotation1"         << YAML::Value << particleSystem->startRotation1;
				out << YAML::Key << "startRotation2"         << YAML::Value << particleSystem->startRotation2;
				out << YAML::Key << "startRotationType"      << YAML::Value << particleSystem->startRotationType;                  
				out << YAML::Key << "flipRotation"           << YAML::Value << particleSystem->flipRotation;                       
				out << YAML::Key << "startColor1"            << YAML::Value << particleSystem->startColor1;
				out << YAML::Key << "startColor2"            << YAML::Value << particleSystem->startColor2;
				out << YAML::Key << "startColorType"         << YAML::Value << particleSystem->startColorType;                     
				out << YAML::Key << "simulationSpace"        << YAML::Value << particleSystem->simulationSpace;                    
				out << YAML::Key << "simulationSpeed"        << YAML::Value << particleSystem->simulationSpeed;                    
				out << YAML::Key << "isDeltaTimeScaled"      << YAML::Value << particleSystem->isDeltaTimeScaled;                  
				out << YAML::Key << "playOnAwake"            << YAML::Value << particleSystem->playOnAwake;		                  
				out << YAML::Key << "maxParticles"           << YAML::Value << particleSystem->maxParticles;                       

				auto& emission = particleSystem->emission;
				out << YAML::Key << "emission" << YAML::Value;
				out << YAML::BeginMap; // emission
				{
					out << YAML::Key << "enabled"              << YAML::Value << emission.enabled;
					out << YAML::Key << "rateOverTime1"        << YAML::Value << emission.rateOverTime1;
					out << YAML::Key << "rateOverTime2"        << YAML::Value << emission.rateOverTime2;
					out << YAML::Key << "rateOverTimeType"     << YAML::Value << emission.rateOverTimeType;
					out << YAML::Key << "rateOverDistance1"    << YAML::Value << emission.rateOverDistance1;
					out << YAML::Key << "rateOverDistance2"    << YAML::Value << emission.rateOverDistance2;
					out << YAML::Key << "rateOverDistanceType" << YAML::Value << emission.rateOverDistanceType;
					auto& bursts = emission.bursts;
					out << YAML::Key << "bursts" << YAML::Value;
					out << YAML::BeginSeq; // bursts
					{
						for (auto& burst : bursts)
						{
							out << YAML::BeginMap;//burst
							{
								out << YAML::Key << "time"        << YAML::Value << burst.time;
								out << YAML::Key << "count1"      << YAML::Value << burst.count1;
								out << YAML::Key << "count2"      << YAML::Value << burst.count2;
								out << YAML::Key << "countType"   << YAML::Value << burst.countType;
								out << YAML::Key << "cycles"      << YAML::Value << burst.cycles;
								out << YAML::Key << "cyclesType"  << YAML::Value << burst.cyclesType;
								out << YAML::Key << "interval"    << YAML::Value << burst.interval;
								out << YAML::Key << "probability" << YAML::Value << burst.probability;
								out << YAML::EndMap;//burst
							}
						}
						out << YAML::EndSeq; // bursts
					}
					out << YAML::EndMap;//emission
				}

				auto& shape = particleSystem->shape;
				out << YAML::Key << "shape" << YAML::Value;
				out << YAML::BeginMap; // shape
				{
				    out << YAML::Key << "enabled"         << YAML::Value << shape.enabled;
				    out << YAML::Key << "shape"           << YAML::Value << (int)shape.shape;
				    out << YAML::Key << "angle"           << YAML::Value << shape.angle;
				    out << YAML::Key << "radius"          << YAML::Value << shape.radius;
				    out << YAML::Key << "radiusThickness" << YAML::Value << shape.radiusThickness;
				    out << YAML::Key << "arc"             << YAML::Value << shape.arc;
				    out << YAML::Key << "boxThickness"    << YAML::Value << shape.boxThickness;
				    out << YAML::Key << "mode"            << YAML::Value << shape.mode;
				    out << YAML::Key << "length"          << YAML::Value << shape.length;
				    out << YAML::Key << "emitFrom"        << YAML::Value << (int)shape.emitFrom;
				    out << YAML::Key << "texture"         << YAML::Value << shape.texture;
				    out << YAML::Key << "position"        << YAML::Value << shape.position;
				    out << YAML::Key << "rotation"        << YAML::Value << shape.rotation;
				    out << YAML::Key << "scale"           << YAML::Value << shape.scale;

					out << YAML::EndMap;//shape
				}


				auto& renderer = particleSystem->renderer;
				out << YAML::Key << "renderer" << YAML::Value;
				out << YAML::BeginMap; // renderer
				{
					out << YAML::Key << "enabled"       << YAML::Value << renderer.enabled;
					out << YAML::Key << "renderMode"    << YAML::Value << (int)renderer.renderMode;
					out << YAML::Key << "meshes"        << YAML::Value << YAML::Flow << renderer.meshes;
					out << YAML::Key << "materialLibraryKey" << YAML::Value << renderer.materialLibraryKey.key;
					out << YAML::EndMap;//renderer
				}

				auto& rotationOverLifetime = particleSystem->rotationOverLifetime;
				out << YAML::Key << "rotationOverLifetime" << YAML::Value;
				out << YAML::BeginMap; // rotationOverLifetime
				{
					out << YAML::Key << "enabled"             << YAML::Value << rotationOverLifetime.enabled;
					out << YAML::Key << "separateAxes"        << YAML::Value << rotationOverLifetime.separateAxes;
					out << YAML::Key << "angularVelocityType" << YAML::Value << rotationOverLifetime.angularVelocityType;
					out << YAML::Key << "angularVelocity1"    << YAML::Value << rotationOverLifetime.angularVelocity1;
					out << YAML::Key << "angularVelocity2"    << YAML::Value << rotationOverLifetime.angularVelocity2;
					out << YAML::Key << "angularVelocity3D1"  << YAML::Value << rotationOverLifetime.angularVelocity3D1;
					out << YAML::Key << "angularVelocity3D2"  << YAML::Value << rotationOverLifetime.angularVelocity3D2;

					out << YAML::EndMap;//rotationOverLifetime
				}

				out << YAML::EndMap; // particleSystemComponent
			}
		};
	}
}