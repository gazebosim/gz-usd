---
orphan: true
---
# Template: Conceptual Data Mapping Document 

```{important}
**{octicon}`tag;1em` Template Version:** {bdg-secondary}`1.0.0` 
<br>**{octicon}`calendar;1em` Last Update:** {bdg-secondary}`2024-09-18`

You can download the Markdown for {download}`this conceptual data mapping document template <./conceptual_data_mapping_template.md>` to use as a starting point for your own document.
```

## Introduction

### Overview
Overview text

### References
This document has been prepared in reference to the software or specification versions listed below. Adjustments or considerations may need to be made for previous or future versions than those referenced in this document.

#### X Reference
| Version | Reference Documents  |
|---|---|    
|   |   |    


#### OpenUSD Reference

| Version | Reference Documents  |
|---|---|    
| 24.08  | [OpenUSD C++ and Schema Documentation](https://openusd.org/release/api/index.html), [OpenUSD Github Repository](https://github.com/PixarAnimationStudios/OpenUSD), [USD Terms and Concepts](https://openusd.org/release/glossary.html)  |     


### General Assumptions and Constraints
Explain if this document focuses only on One-Way mapping (X->OpenUSD) or Two-Way.
Any decision that has a ripple effect on lots of concepts could be briefly noted here and then described in more detail in an Appendix (e.g. Transcoding). One advantage of utilizing Appendices in this way is that we can potentially explain a pattern once and reuse it in multiple Conceptual Data Mapping documents. 

### Definitions, Acronyms, Abbreviations
Define terms used in this document. They could come from one of the source formats and be presented here for reader convenience. No need for establishing mappings for these in this table.

| Term or Abbreviation | Description |
|---|---|    
| scenegraph  |  A data structure that organizes a graphical scene's logical and spatial representation as a tree or graph of nodes. |     
|   |   |

## Concepts
You can include introduction/overview text here. Then followed by a table showing high-level concept mappings. List all concepts from X even if there is no direct mapping or support for the feature in OpenUSD. If the concept doesn’t exist or map to OpenUSD, leave the OpenUSD cell blank and briefly explain in the Description.

Not required, but beneficial for Two-Way translation, also list all OpenUSD concepts even if there is no direct mapping or support for a feature in X. If the concept doesn’t exist or map to X, leave the X cell blank and briefly explain in the Description. You can link the OpenUSD cell to a drilldown section to explain the limitation in more detail.

| SDFormat | OpenUSD | Description |
|----------|---------|-------------|
|[//inertial](http://sdformat.org/spec?ver=1.11&elem=link#link_inertial) | [MassAPI](https://openusd.org/release/api/class_usd_physics_mass_a_p_i.html) | Inertial components of a Link |
|[//inertial/pose](http://sdformat.org/spec?ver=1.11&elem=link#inertial_pose) x,y,z components | `physics:centerofmass` | Subcomponent of Inertial. Represents the position for the center of mass for the link. |
|[//inertial/mass](http://sdformat.org/spec?ver=1.11&elem=link#inertial_mass) | `physics:mass` | Subcomponent of Inertial. Mass of inertial element. |
|[//inertial/inertia](http://sdformat.org/spec?ver=1.11&elem=link#inertial_inertia) | `physics:diagonalInertia`, `physics:principalAxes` | Subcomponent of Inertial. Moment of inertia in diagonal half matrix (xx, xy, xz, yy, yz, zz components) |

### ConceptA
Each high-level concept from the table should link to subsections like this one where you can go into more detail and then breakdown how you would encode this concept using 

#### Properties
| X | OpenUSD | Description |
|---|---|---|
|[PropA](#property-propa)|propA| |
|PropB|propB| |

##### Property: PropA
More info about the property and any data transformations that should be applied.
|  | Name | Data Type |
|---|---|---|
|X|PropA| float |
|OpenUSD|propA| float |

#### Metadata

#### Composition

### UsdConceptB
This is an example of a concept that only exists in OpenUSD and it's optionally documented for the purpose of Two-Way translation.You can explain the limitation in more detail and perhaps offer a solution to be able to roundtrip this data without data loss.

#### Properties

#### Metadata

#### Composition

## Appendices

### Appendix A: Topic A

### Appendix B: Topic B
