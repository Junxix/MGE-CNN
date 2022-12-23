import os
import sys
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from importlib import import_module
import copy

import torchvision
import torchvision.models as models

from .base import BaseNet
from .grad_cam import GradCam

import pdb

def l2_norm_v2(input):
    input_size = input.size()
    _output = input/(torch.norm(input, p=2, dim=-1, keepdim=True))
    output = _output.view(input_size)
    return output

class Classifier(nn.Module):
    def __init__(self, in_panel, out_panel, bias=False):
        super(Classifier, self).__init__()
        self.fc = nn.Linear(in_panel, out_panel, bias=bias)

    def forward(self, input):
        logit = self.fc(input)
        if logit.dim()==1:
            logit =logit.unsqueeze(0)
        return logit

class augNet(nn.module):
    def __init__(self, opt=None):
        super(augNet, self).__init__()
        num_classes = opt.num_classes
        basenet = getattr(import_module('torchvision.models'), opt.arch)
        basenet = basenet(pretrained=True)
        self.conv4 = nn.Sequential(*list(basenet.children())[:-3])
        self.conv5 = nn.Sequential(*list(basenet.children())[-3])
        self.pool = nn.AdaptiveAvgPool2d(1)
        self.classifier = Classifier(2048, num_classes, bias=True)

        self.conv6 = nn.conv2(1024, 10 * num_classes, 1, 1, 1)
        self.pool_max = nn.AdaptiveMaxPool2d(1)
        self.cls_part = Classifier(10 * num_classes, num_classes, bias=True)
        self.cls_cat = Classifier(2048 + 10 * num_classes, num_classes, bias=True)

    def forward(self, x, y=None):
        batch_size = x.size(0)
        conv4 = self.conv4(x) # [batch_size, 1024, h, w]
        conv5 = self.conv4(conv4) # [batch_size, 2048, h, w]
        conv5_pool = self.pool(conv5).view(batch_size, -1) # [batch_size, num_classes]
        logits = self.classifier(conv5_pool) # [batch_size, num_classes]

        conv6_pool = self.pool_max(F.relu(self.conv6(conv4.detach()))).view(batch_size, -1) # [batch_size, 10 * num_classes]
        cat_pool = torch.cat((10 * l2_norm_v2(conv5_pool.detach()), 10 * l2_norm_v2(conv6_pool.detach())), dim=1) #[batch_size, 10 * num_classes + 2048]

        logits_max = self.cls_part(conv6_pool) # [batch_size, num_classes]
        logits_cat = self.cls_cat(cat_pool) #[batch_size, num_classes]

        logits_list = [logits, logits_max, logits_cat]

        outputs = {'logits':logits_list}
        return outputs
    
    def get_params(self, prefix='extractor'):
        extractor_params = list(self.conv5.parameters()) +\
                           list(self.conv4.parameters()) 
        extractor_params_ids = list(map(id, extractor_params))
        classifier_params = filter(lambda p: id(p) not in extractor_params_ids, self.parameters())

        if prefix in ['extractor', 'extract']:
            return extractor_params
        elif prefix in ['classifier']:
            return classifier_params