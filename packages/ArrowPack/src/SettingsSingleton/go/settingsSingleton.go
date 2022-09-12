package SettingsSingleton

import (
	"regexp"
	"strings"

	"github.com/asaskevich/govalidator"
)

type SettingsSingleton struct {
	StringValues map[string]string
	BooleanValues map[string]bool
	IntegerValues map[string]int
    FloatValues map[string]float64
}

func InitSettingsSingleton(StringFormat string) *SettingsSingleton {
	Settings := new(SettingsSingleton)
	re := regexp.MustCompile("::.")
	Values := re.FindAllStringSubmatch(StringFormat, -1)
	
	for _, v := range Values {
		splitValues := strings.Split(v, ":")
		if govalidator.IsInt(splitValues[1]) {
			Settings.IntegerValues[splitValues[0]] = govalidator.ToInt(splitValues[1])
		}
	}

    return &Settings
}
