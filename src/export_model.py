import torch
import torch.nn as nn

# Your CNN architecture
class DefectCNN(nn.Module):
    def __init__(self, num_classes=5):
        super().__init__()
        self.conv1 = nn.Conv2d(1, 32, 3, padding=1)
        self.pool = nn.MaxPool2d(2)
        self.conv2 = nn.Conv2d(32, 64, 3, padding=1)
        self.fc = nn.Linear(64 * 56 * 56, num_classes)
    
    def forward(self, x):
        x = self.pool(torch.relu(self.conv1(x)))
        x = self.pool(torch.relu(self.conv2(x)))
        x = x.view(x.size(0), -1)
        return self.fc(x)

# Load trained model and export
model = DefectCNN()
model.load_state_dict(torch.load('trained_model.pth'))
model.eval()

dummy_input = torch.randn(1, 1, 224, 224)
torch.onnx.export(model, dummy_input, "defect_model.onnx")