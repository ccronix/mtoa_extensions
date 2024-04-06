import mtoa.ui.ae.templates as templates
from mtoa.ui.ae.templates import AttributeTemplate


class DemoTemplate(AttributeTemplate):
    def setup(self):
        pass


templates.registerTranslatorUI(DemoTemplate, "mesh", "demo_translator")
