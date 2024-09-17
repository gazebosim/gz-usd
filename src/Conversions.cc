/*
 * Copyright 2022 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "Conversions.hh"

#include <gz/common/Pbr.hh>

#include "sdf/Pbr.hh"

namespace gz
{
  inline namespace GZ_USD_VERSION_NAMESPACE {
  //
  namespace usd
  {
  sdf::Material convert(const gz::common::Material *_in)
  {
    sdf::Material out;
    out.SetEmissive(_in->Emissive());
    out.SetDiffuse(_in->Diffuse());
    out.SetSpecular(_in->Specular());
    out.SetAmbient(_in->Ambient());
    out.SetRenderOrder(_in->RenderOrder());
    out.SetLighting(_in->Lighting());
    out.SetDoubleSided(_in->TwoSidedEnabled());
    const gz::common::Pbr * pbr = _in->PbrMaterial();
    if (pbr != nullptr)
    {
      out.SetNormalMap(pbr->NormalMap());
      sdf::Pbr pbrOut;
      sdf::PbrWorkflow pbrWorkflow;
      if (!pbr->AlbedoMap().empty())
      {
        pbrWorkflow.SetAlbedoMap(pbr->AlbedoMap());
      }
      else
      {
        pbrWorkflow.SetAlbedoMap(_in->TextureImage());
      }
      pbrWorkflow.SetMetalnessMap(pbr->MetalnessMap());
      pbrWorkflow.SetEmissiveMap(pbr->EmissiveMap());
      pbrWorkflow.SetRoughnessMap(pbr->RoughnessMap());
      pbrWorkflow.SetSpecularMap(pbr->SpecularMap());
      pbrWorkflow.SetEnvironmentMap(pbr->EnvironmentMap());
      pbrWorkflow.SetAmbientOcclusionMap(pbr->AmbientOcclusionMap());
      pbrWorkflow.SetLightMap(pbr->LightMap());
      pbrWorkflow.SetRoughness(pbr->Roughness());
      pbrWorkflow.SetGlossiness(pbr->Glossiness());
      pbrWorkflow.SetMetalness(pbr->Metalness());

      if (pbr->NormalMapType() == gz::common::NormalMapSpace::TANGENT)
      {
        pbrWorkflow.SetNormalMap(
          pbr->NormalMap(), sdf::NormalMapSpace::TANGENT);
      }
      else
      {
        pbrWorkflow.SetNormalMap(
          pbr->NormalMap(), sdf::NormalMapSpace::OBJECT);
      }

      if (pbr->Type() == gz::common::PbrType::SPECULAR)
      {
        pbrWorkflow.SetType(sdf::PbrWorkflowType::SPECULAR);
        pbrOut.SetWorkflow(sdf::PbrWorkflowType::SPECULAR, pbrWorkflow);
      }
      else
      {
        pbrWorkflow.SetType(sdf::PbrWorkflowType::METAL);
        pbrOut.SetWorkflow(sdf::PbrWorkflowType::METAL, pbrWorkflow);
      }
      out.SetPbrMaterial(pbrOut);
    }
    else if (!_in->TextureImage().empty())
    {
      sdf::Pbr pbrOut;
      sdf::PbrWorkflow pbrWorkflow;
      pbrWorkflow.SetAlbedoMap(_in->TextureImage());
      pbrOut.SetWorkflow(sdf::PbrWorkflowType::SPECULAR, pbrWorkflow);
      out.SetPbrMaterial(pbrOut);
    }

    return out;
  }

  void convert(const sdf::Material &_in, gz::common::Material &_out)
  {
    _out.SetEmissive(_in.Emissive());
    _out.SetDiffuse(_in.Diffuse());
    _out.SetSpecular(_in.Specular());
    _out.SetAmbient(_in.Ambient());
    _out.SetRenderOrder(_in.RenderOrder());
    _out.SetLighting(_in.Lighting());
    _out.SetAlphaFromTexture(false, 0.5, _in.DoubleSided());

    const sdf::Pbr * pbr = _in.PbrMaterial();
    if (pbr != nullptr)
    {
      gz::common::Pbr pbrOut;

      const sdf::PbrWorkflow * pbrWorkflow =
        pbr->Workflow(sdf::PbrWorkflowType::METAL);
      if (pbrWorkflow)
      {
        pbrOut.SetType(gz::common::PbrType::METAL);
      }
      else
      {
        pbrWorkflow = pbr->Workflow(sdf::PbrWorkflowType::SPECULAR);
        if (pbrWorkflow)
        {
          pbrOut.SetType(gz::common::PbrType::SPECULAR);
        }
      }
      if (pbrWorkflow != nullptr)
      {
        pbrOut.SetAlbedoMap(pbrWorkflow->AlbedoMap());
        pbrOut.SetMetalnessMap(pbrWorkflow->MetalnessMap());
        pbrOut.SetEmissiveMap(pbrWorkflow->EmissiveMap());
        pbrOut.SetRoughnessMap(pbrWorkflow->RoughnessMap());
        pbrOut.SetSpecularMap(pbrWorkflow->SpecularMap());
        pbrOut.SetEnvironmentMap(pbrWorkflow->EnvironmentMap());
        pbrOut.SetAmbientOcclusionMap(pbrWorkflow->AmbientOcclusionMap());
        pbrOut.SetLightMap(pbrWorkflow->LightMap());
        pbrOut.SetRoughness(pbrWorkflow->Roughness());
        pbrOut.SetGlossiness(pbrWorkflow->Glossiness());
        pbrOut.SetMetalness(pbrWorkflow->Metalness());

        if (pbrWorkflow->NormalMapType() == sdf::NormalMapSpace::TANGENT)
        {
          pbrOut.SetNormalMap(
            pbrWorkflow->NormalMap(),
            gz::common::NormalMapSpace::TANGENT);
        }
        else if (pbrWorkflow->NormalMapType() == sdf::NormalMapSpace::OBJECT)
        {
          pbrOut.SetNormalMap(
            pbrWorkflow->NormalMap(),
            gz::common::NormalMapSpace::OBJECT);
        }
      }
      _out.SetPbrMaterial(pbrOut);
    }
  }
  }
  }
}
