---
layout: default
title: Common Input
nav_order: 7
has_children: true
permalink: /docs/input
---

# Common Input

* Create a data asset of type `CommonInputActionDataBase`and add entries for `DefaultForward`, `DefaultBack` and `GenericAccept`
* Create BP of type `CommonUIInputData` and link to the data table created above for `DefaultClick` (with RowName to `DefaultForward`) and `DefaultBack` (with RowName to `DefaultBack`)